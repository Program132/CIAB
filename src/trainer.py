from collections import Counter
from concurrent.futures import ProcessPoolExecutor, as_completed
import json
import os
from pathlib import Path
import sys
from tqdm import tqdm

sys.path.append(str(Path(__file__).resolve().parent))
from config import *

ROOT_DIR = Path(__file__).resolve().parent.parent
DATASETS_DIR = ROOT_DIR / DEFAULT_DATASETS_FOLDER
DATABASE_DIR = ROOT_DIR / DATABASE_FOLDER
INDEX_FILE = DATABASE_DIR / "files_index.bin"
STATE_FILE = DATABASE_DIR / "state.json"
TOKENIZER_FILE = DATABASE_DIR / "tokenizer.bin"
CHUNKS_DIR = DATABASE_DIR / "trainer_chunks"

BATCH_SIZE = 2000

_worker_vocab = None
_worker_bos_id = 1
_worker_eos_id = 2
_worker_unk_id = 3


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


def init_worker(tokenizer_path):
    global _worker_vocab, _worker_bos_id, _worker_eos_id, _worker_unk_id
    vocab = {}
    with open(tokenizer_path, "rb") as f:
        for line in f:
            line_str = line.decode("utf-8", errors="ignore").strip()
            if not line_str:
                continue
            parts = line_str.split("|", 1)
            if len(parts) == 2:
                vocab[parts[1]] = int(parts[0])
    _worker_vocab = vocab
    _worker_bos_id = vocab.get(TOKEN_START_SENTENCE, 1)
    _worker_eos_id = vocab.get(TOKEN_END_SENTENCE, 2)
    _worker_unk_id = vocab.get(TOKEN_UNKNOWN_WORD, 3)


def process_single_file(args):
    rel_path, root_dir = args
    file_path = root_dir / rel_path
    trigrams = Counter()
    if file_path.exists() and _worker_vocab is not None:
        try:
            text = file_path.read_text(encoding="utf-8", errors="ignore")
            raw_words = WORD_REGEX.findall(text)
            if raw_words:
                token_ids = [_worker_vocab.get(w.lower(), _worker_unk_id) for w in raw_words]
                sequence = [_worker_bos_id, _worker_bos_id] + token_ids + [_worker_eos_id]
                for i in range(len(sequence) - 2):
                    trigrams[(sequence[i], sequence[i + 1], sequence[i + 2])] += 1
        except Exception:
            pass
    return trigrams


def trainer():
    if not INDEX_FILE.exists():
        print(f"Erreur : Le fichier d'index {INDEX_FILE} n'existe pas.")
        return

    if not TOKENIZER_FILE.exists():
        print(f"Erreur : Le fichier {TOKENIZER_FILE} n'existe pas.")
        return

    DATABASE_DIR.mkdir(parents=True, exist_ok=True)
    CHUNKS_DIR.mkdir(parents=True, exist_ok=True)
    state = load_state()
    last_id_done = state.get("last_trainer_file_id_done", -1)
    start_id = last_id_done + 1

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
    chunk_counter = len(list(CHUNKS_DIR.glob("chunk_*.bin")))

    try:
        max_workers = os.cpu_count() or 4
        print(f"Utilisation de {max_workers} cœurs CPU en parallèle.")

        with tqdm(total=total_files, desc="Entraînement", unit=" fichiers", dynamic_ncols=True) as pbar:
            with ProcessPoolExecutor(max_workers=max_workers, initializer=init_worker, initargs=(TOKENIZER_FILE,)) as executor:
                for i in range(0, total_files, BATCH_SIZE):
                    batch = entries[i:i + BATCH_SIZE]
                    futures = {
                        executor.submit(process_single_file, (rel_path, ROOT_DIR)): file_id
                        for file_id, rel_path in batch
                    }

                    batch_max_file_id = last_id_done
                    batch_trigrams = Counter()

                    for future in as_completed(futures):
                        file_id = futures[future]
                        batch_max_file_id = max(batch_max_file_id, file_id)
                        try:
                            file_trigrams = future.result()
                            batch_trigrams.update(file_trigrams)
                        except Exception:
                            pass
                        pbar.update(1)

                    if batch_trigrams:
                        chunk_file = CHUNKS_DIR / f"chunk_{chunk_counter:05d}.bin"
                        with open(chunk_file, "wb") as cf:
                            lines = [f"{w1}|{w2}|{w3}|{freq}\n".encode("utf-8") for (w1, w2, w3), freq in batch_trigrams.items()]
                            cf.writelines(lines)
                        chunk_counter += 1

                    last_processed_file_id = batch_max_file_id
                    state["last_trainer_file_id_done"] = last_processed_file_id
                    save_state(state)
                    pbar.set_postfix(chunks=chunk_counter, refresh=False)

    except KeyboardInterrupt:
        print("\nInterruption détectée.")
    finally:
        state["last_trainer_file_id_done"] = last_processed_file_id
        save_state(state)
        print(f"\nArrêt au fichier ID {last_processed_file_id}. Chunks sauvegardés : {chunk_counter}.")


if __name__ == "__main__":
    trainer()