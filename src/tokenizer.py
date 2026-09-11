import json
import sys
from pathlib import Path
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor, as_completed
import os

sys.path.append(str(Path(__file__).resolve().parent))
from config import *

ROOT_DIR = Path(__file__).resolve().parent.parent
DATASETS_DIR = ROOT_DIR / DEFAULT_DATASETS_FOLDER
DATABASE_DIR = ROOT_DIR / DATABASE_FOLDER
INDEX_FILE = DATABASE_DIR / "files_index.bin"
STATE_FILE = DATABASE_DIR / "state.json"
TOKENIZER_FILE = DATABASE_DIR / "tokenizer.bin"

SAVE_INTERVAL = 5000
BATCH_SIZE = 2000


def load_state():
    if STATE_FILE.exists():
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            pass
    return {"last_file_id_done": -1, "last_trainer_file_id_done": -1}


def save_state(state):
    DATABASE_DIR.mkdir(parents=True, exist_ok=True)
    with open(STATE_FILE, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=4)


def process_single_file(args):
    rel_path, root_dir = args
    file_path = root_dir / rel_path
    tokens_found = set()
    if file_path.exists():
        try:
            text = file_path.read_text(encoding="utf-8", errors="ignore")
            for token in WORD_REGEX.findall(text):
                tokens_found.add(token.lower())
        except Exception:
            pass
    return tokens_found


def tokenizer():
    if not INDEX_FILE.exists():
        print(f"Erreur : Le fichier d'index {INDEX_FILE} n'existe pas.")
        return

    DATABASE_DIR.mkdir(parents=True, exist_ok=True)
    state = load_state()
    last_id_done = state.get("last_file_id_done", -1)
    start_id = last_id_done + 1

    vocab = set()
    current_word_id = 1

    if start_id > 0 and TOKENIZER_FILE.exists():
        print(f"Reprise à partir du fichier ID {start_id}. Chargement du vocabulaire...")
        with open(TOKENIZER_FILE, "rb") as vf:
            for line in vf:
                line_str = line.decode("utf-8", errors="ignore").strip()
                if not line_str:
                    continue
                parts = line_str.split("|", 1)
                if len(parts) == 2:
                    wid_str, w = parts
                    vocab.add(w)
                    wid = int(wid_str)
                    if wid >= current_word_id:
                        current_word_id = wid + 1
        print(f"Vocabulaire chargé : {len(vocab)} tokens uniques.")
        open_mode = "ab"
    else:
        start_id = 0
        open_mode = "wb"
        print("Démarrage d'un nouveau vocabulaire.")

    print("Lecture de l'index...")
    entries = []
    with open(INDEX_FILE, "rb") as f:
        for current_id, line in enumerate(f):
            line_str = line.decode("utf-8", errors="ignore").strip()
            if not line_str:
                continue
            file_id_str, rel_path = line_str.split("|", 1)
            file_id = int(file_id_str)
            if file_id >= start_id:
                entries.append((file_id, rel_path))

    total_files = len(entries)
    if total_files == 0:
        print("Aucun fichier à traiter.")
        return

    last_processed_file_id = last_id_done

    try:
        with open(TOKENIZER_FILE, open_mode) as out_f:
            if open_mode == "wb":
                special_tokens = [TOKEN_START_SENTENCE, TOKEN_END_SENTENCE, TOKEN_UNKNOWN_WORD]
                special_lines = []
                for st in special_tokens:
                    vocab.add(st)
                    special_lines.append(f"{current_word_id}|{st}\n".encode("utf-8"))
                    current_word_id += 1
                out_f.writelines(special_lines)

            max_workers = os.cpu_count() or 4
            print(f"Utilisation de {max_workers} cœurs CPU en parallèle.")

            with tqdm(total=total_files, desc="Tokenisation", unit=" fichiers", dynamic_ncols=True) as pbar:
                with ProcessPoolExecutor(max_workers=max_workers) as executor:
                    for i in range(0, total_files, BATCH_SIZE):
                        batch = entries[i:i + BATCH_SIZE]
                        futures = {
                            executor.submit(process_single_file, (rel_path, ROOT_DIR)): file_id
                            for file_id, rel_path in batch
                        }

                        batch_max_file_id = last_id_done
                        for future in as_completed(futures):
                            file_id = futures[future]
                            batch_max_file_id = max(batch_max_file_id, file_id)
                            try:
                                file_tokens = future.result()
                                new_vocab_lines = []
                                for word in file_tokens:
                                    if word not in vocab:
                                        vocab.add(word)
                                        new_vocab_lines.append(f"{current_word_id}|{word}\n".encode("utf-8"))
                                        current_word_id += 1
                                if new_vocab_lines:
                                    out_f.writelines(new_vocab_lines)
                            except Exception:
                                pass

                            pbar.update(1)

                        last_processed_file_id = batch_max_file_id
                        state["last_file_id_done"] = last_processed_file_id
                        save_state(state)
                        pbar.set_postfix(vocab_size=len(vocab), refresh=False)

    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde...")
    finally:
        state["last_file_id_done"] = last_processed_file_id
        save_state(state)
        print(f"\nArrêt au fichier ID {last_processed_file_id}. Vocabulaire final : {len(vocab)} tokens.")


if __name__ == "__main__":
    tokenizer()