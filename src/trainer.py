import json
import sqlite3
from collections import Counter
from pathlib import Path
import sys
from tqdm import tqdm

sys.path.append(str(Path(__file__).resolve().parent))
from config import (
    DEFAULT_DATABASE_FILE,
    DEFAULT_DATASETS_FOLDER,
    WORD_REGEX,
    TOKEN_START_SENTENCE,
    TOKEN_END_SENTENCE,
    TOKEN_UNKNOWN_WORD,
)

ROOT_DIR = Path(__file__).resolve().parent.parent

DATASETS_DIR = ROOT_DIR / DEFAULT_DATASETS_FOLDER
INDEX_FILE = ROOT_DIR / "files_index.txt"
STATE_FILE = ROOT_DIR / "state.json"
DATABASE_FILE = ROOT_DIR / DEFAULT_DATABASE_FILE

BATCH_FILES = 50


def init_db(connection: sqlite3.Connection):
    with connection:
        connection.execute("PRAGMA journal_mode = WAL;")
        connection.execute("PRAGMA synchronous = NORMAL;")
        connection.execute("""
            CREATE TABLE IF NOT EXISTS ngrams (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_id_1 INTEGER NOT NULL,
                word_id_2 INTEGER NOT NULL,
                word_id_3 INTEGER NOT NULL,
                frequency INTEGER DEFAULT 1,
                UNIQUE(word_id_1, word_id_2, word_id_3)
            )
        """)
        connection.execute("""
            CREATE INDEX IF NOT EXISTS idx_ngrams_prefix
            ON ngrams (word_id_1, word_id_2)
        """)


def load_vocab(connection: sqlite3.Connection):
    cursor = connection.cursor()
    cursor.execute("SELECT word, id FROM tokenizer")
    vocab = {row[0]: row[1] for row in cursor.fetchall()}

    bos_id = vocab.get(TOKEN_START_SENTENCE, 1)
    eos_id = vocab.get(TOKEN_END_SENTENCE, 2)
    unk_id = vocab.get(TOKEN_UNKNOWN_WORD, 3)

    return vocab, bos_id, eos_id, unk_id


def save_state(last_id: int):
    state = {}
    if STATE_FILE.exists():
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                state = json.load(f)
        except Exception:
            state = {}
    state["last_trainer_file_id_done"] = last_id
    with open(STATE_FILE, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=4)


def flush_ngrams(connection: sqlite3.Connection, batch_counts: Counter):
    if not batch_counts:
        return
    records = [
        (w1, w2, w3, freq)
        for (w1, w2, w3), freq in batch_counts.items()
    ]
    with connection:
        connection.executemany("""
            INSERT INTO ngrams (word_id_1, word_id_2, word_id_3, frequency)
            VALUES (?, ?, ?, ?)
            ON CONFLICT(word_id_1, word_id_2, word_id_3)
            DO UPDATE SET frequency = frequency + excluded.frequency
        """, records)
    batch_counts.clear()


def trainer():
    if not STATE_FILE.exists() or not INDEX_FILE.exists():
        print("Erreur : Veuillez exécuter preprocessor.py au préalable.")
        return

    if not DATABASE_FILE.exists():
        print("Erreur : La base de données n'existe pas. Veuillez exécuter tokenizer.py au préalable.")
        return

    with open(STATE_FILE, "r", encoding="utf-8") as f:
        state = json.load(f)
    last_id = state.get("last_trainer_file_id_done", -1)

    connection = sqlite3.connect(DATABASE_FILE)
    init_db(connection)

    print("Chargement du vocabulaire en mémoire...")
    vocab, bos_id, eos_id, unk_id = load_vocab(connection)
    if not vocab:
        print("Erreur : La table 'tokenizer' est vide. Veuillez exécuter tokenizer.py au préalable.")
        connection.close()
        return
    print(f"Vocabulaire chargé ({len(vocab)} tokens).")

    with open(INDEX_FILE, "r", encoding="utf-8") as f:
        total_files = sum(1 for line in f if line.strip())

    current_id = last_id
    batch_counts = Counter()
    processed_in_batch = 0

    try:
        with open(INDEX_FILE, "r", encoding="utf-8") as f:
            with tqdm(
                total=total_files,
                initial=max(0, last_id + 1),
                desc="Entraînement (Trigrams)",
                unit=" fichiers",
                dynamic_ncols=True
            ) as pbar:
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

                        raw_words = WORD_REGEX.findall(text)
                        if raw_words:
                            token_ids = [vocab.get(w.lower(), unk_id) for w in raw_words]
                            sequence = [bos_id, bos_id] + token_ids + [eos_id]

                            for i in range(len(sequence) - 2):
                                trigram = (sequence[i], sequence[i + 1], sequence[i + 2])
                                batch_counts[trigram] += 1

                    current_id = file_id
                    processed_in_batch += 1
                    pbar.update(1)

                    if processed_in_batch >= BATCH_FILES:
                        flush_ngrams(connection, batch_counts)
                        save_state(current_id)
                        processed_in_batch = 0

    except KeyboardInterrupt:
        print("\nInterruption détectée. Sauvegarde de la base et de l'état...")
    finally:
        flush_ngrams(connection, batch_counts)
        save_state(current_id)
        connection.close()
        print(f"\nEntraînement arrêté au fichier ID {current_id}. Base de données fermée.")


if __name__ == "__main__":
    trainer()
