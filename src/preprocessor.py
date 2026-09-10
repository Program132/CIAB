"""
Create a file "files_index.txt" and "state.json"

files_index.txt format:
0|datasets/wikipedia/...
1|datasets/wikipedia/...
...
54564|datasets/wikipedia/...


state.json format:
{
    "last_file_id_done": -1,
    "last_trainer_file_id_done": -1
}
"""

import os
import json
import sys
from pathlib import Path
from tqdm import tqdm

sys.path.append(str(Path(__file__).resolve().parent))
from config import DEFAULT_DATASETS_FOLDER

ROOT_DIR = Path(__file__).resolve().parent.parent
DATASETS_DIR = ROOT_DIR / DEFAULT_DATASETS_FOLDER
INDEX_FILE = ROOT_DIR / "files_index.txt"
STATE_FILE = ROOT_DIR / "state.json"


def generate_index_and_state():
    if not DATASETS_DIR.exists() or not DATASETS_DIR.is_dir():
        print(f"Erreur : Le dossier {DATASETS_DIR} n'existe pas.")
        return

    root_str = str(ROOT_DIR)
    datasets_str = str(DATASETS_DIR)

    count = 0
    with open(INDEX_FILE, "w", encoding="utf-8") as f:
        with tqdm(desc="Indexation des fichiers", unit=" fichiers") as pbar:
            for root, dirs, files in os.walk(datasets_str):
                dirs.sort()
                files.sort()
                for filename in files:
                    full_path = os.path.join(root, filename)
                    rel_path = os.path.relpath(full_path, root_str).replace("\\", "/")
                    f.write(f"{count}|{rel_path}\n")
                    count += 1
                    pbar.update(1)

    print(f"\nIndex généré avec succès : {count} fichiers indexés dans '{INDEX_FILE.name}'.")

    if not STATE_FILE.exists():
        initial_state = {
            "last_file_id_done": -1,
            "last_trainer_file_id_done": -1
        }
        with open(STATE_FILE, "w", encoding="utf-8") as f:
            json.dump(initial_state, f, indent=4)
        print(f"Fichier d'état initialisé dans '{STATE_FILE.name}'.")


if __name__ == "__main__":
    generate_index_and_state()
