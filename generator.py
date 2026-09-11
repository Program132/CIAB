from src.config import DEFAULT_DATABASE_FILE, WORD_REGEX
import random
import sqlite3
import sys

def generator(db: str = DEFAULT_DATABASE_FILE, request: str = ""):
    if isinstance(request, list):
        request = " ".join(request)

    conn = sqlite3.connect(db)
    cursor = conn.cursor()

    words = WORD_REGEX.findall(request.lower()) if request else []
    token_ids = []
    for w in words:
        cursor.execute("SELECT id FROM tokenizer WHERE word = ?", (w,))
        row = cursor.fetchone()
        if row:
            token_ids.append(row[0])

    if not token_ids:
        w1, w2 = 1, 1
    elif len(token_ids) == 1:
        w1, w2 = 1, token_ids[0]
    else:
        w1, w2 = token_ids[-2], token_ids[-1]

    result_ids = list(token_ids)

    for _ in range(50):
        cursor.execute(
            "SELECT word_id_3, frequency FROM ngrams WHERE word_id_1 = ? AND word_id_2 = ?",
            (w1, w2),
        )
        rows = cursor.fetchall()
        if not rows:
            break

        next_id = random.choices([r[0] for r in rows], weights=[r[1] for r in rows])[0]
        if next_id == 2:
            break

        result_ids.append(next_id)
        w1, w2 = w2, next_id

    output_words = []
    for tid in result_ids:
        cursor.execute("SELECT word FROM tokenizer WHERE id = ?", (tid,))
        row = cursor.fetchone()
        if row:
            output_words.append(row[0])

    conn.close()

    output = " ".join(output_words)
    print(output)
    return output

if __name__ == '__main__':
    args = sys.argv
    db = args[1] if len(args) > 1 else DEFAULT_DATABASE_FILE
    if len(args) < 3:
        req = input("IA Request: ")
    else:
        req = " ".join(args[2:])
    generator(db=db, request=req)
