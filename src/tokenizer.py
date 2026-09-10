import json
import sqlite3
from config import DEFAULT_DATABASE_FILE

def indexer():
    with open("state.json", "r") as statej:
        statef = json.load(statej)

    last_id = statef["last_file_id_done"]
    connection = sqlite3.connect(DEFAULT_DATABASE_FILE)
    cursor = connection.cursor()

    if last_id is None or last_id == -1:
        # first time
        ...
    else:
        # reprise de l'indexation
        ...