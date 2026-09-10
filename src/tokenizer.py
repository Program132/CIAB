import json
import sqlite3
from pathlib import Path
import sys
from tqdm import tqdm

sys.path.append(str(Path(__file__).resolve().parent))
from config import DEFAULT_DATABASE_FILE, DEFAULT_DATASETS_FOLDER, WORD_REGEX, TOKEN_START_SENTENCE, TOKEN_END_SENTENCE, TOKEN_UNKNOWN_WORD

ROOT_DIR = Path(__file__).resolve().parent.parent

DATASETS_DIR = ROOT_DIR / DEFAULT_DATASETS_FOLDER
INDEX_FILE = ROOT_DIR / "files_index.txt"
STATE_FILE = ROOT_DIR / "state.json"
DATABASE_FILE = ROOT_DIR / DEFAULT_DATABASE_FILE

def init_db(connection: sqlite3.Connection):
    connection.execute("PRAGMA journal_mode = WAL;")
    connection.execute("PRAGMA synchronous = NORMAL;")
    connection.execute("PRAGMA cache_size = -64000;")
    connection.execute("PRAGMA temp_store = MEMORY;")

    with connection:
        connection.execute("""
            CREATE TABLE IF NOT EXISTS tokenizer (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word TEXT UNIQUE NOT NULL
            )
        """)

        special_tokens = [
            (1, TOKEN_START_SENTENCE),
            (2, TOKEN_END_SENTENCE),
            (3, TOKEN_UNKNOWN_WORD),
        ]
        connection.executemany(
            "INSERT OR IGNORE INTO tokenizer (id, word) VALUES (?, ?)",
            special_tokens
        )

def save_state(last_id: int):
    state = {}
    if STATE_FILE.exists():
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                state = json.load(f)
        except Exception:
            state = {}
    state["last_file_id_done"] = last_id
    with open(STATE_FILE, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=4)


def tokenizer():
    if not STATE_FILE.exists() or not INDEX_FILE.exists():
        print("Erreur : Veuillez exécuter preprocessor.py au préalable.")
        return

    with open(STATE_FILE, "r", encoding="utf-8") as f:
        state = json.load(f)
    last_id = state.get("last_file_id_done", -1)

    connection = sqlite3.connect(DATABASE_FILE)
    init_db(connection)

    with open(INDEX_FILE, "r", encoding="utf-8") as f:
        total_files = sum(1 for line in f if line.strip())

    current_id = last_id
    try:
        with open(INDEX_FILE, "r", encoding="utf-8") as f:
            with tqdm(total=total_files, initial=max(0, last_id + 1), desc="Tokenisation", unit=" fichiers", dynamic_ncols=True) as pbar:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    file_id_str, rel_path = line.split("|", 1)
                    file_id = int(file_id_str)

                    if file_id <= last_id:
                        continue

                    file_path = ROOT_DIR / rel_path
                    if file_path.exists():
                        with open(file_path, "r", encoding="utf-8", errors="ignore") as content_file:
                            text = content_file.read()
                            words = {(w.lower(),) for w in WORD_REGEX.findall(text)}

                            with connection:
                                connection.executemany("INSERT OR IGNORE INTO tokenizer (word) VALUES (?)", words)

                    current_id = file_id
                    pbar.update(1)

                    if current_id % 100 == 0:
                        save_state(current_id)

    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde de l'état...")
    finally:
        save_state(current_id)
        connection.close()
        print(f"\nIndexation arrêtée au fichier ID {current_id}. Base de données fermée.")

if __name__ == "__main__":
    tokenizer()
