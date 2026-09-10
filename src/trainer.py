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
    with connection:
        connection.execute("""
            CREATE TABLE IF NOT EXISTS ngrams (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_id_1 INTEGER,
                word_id_2 INTEGER,
                word_id_3 INTEGER,
                frequency INTEGER DEFAULT 1,
                UNIQUE(word_id_1, word_id_2, word_id_3)
            )
        """)

def trainer():
    connection = sqlite3.connect(DATABASE_FILE)
    init_db(connection)

    init_db(connection)

if __name__ == "__main__":
    trainer()