import random
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parent))
from config import DATABASE_FOLDER, WORD_REGEX, TOKEN_START_SENTENCE, TOKEN_END_SENTENCE

ROOT_DIR       = Path(__file__).resolve().parent.parent
DATABASE_DIR   = ROOT_DIR / DATABASE_FOLDER
TOKENIZER_FILE = DATABASE_DIR / "tokenizer.bin"
TRAINER_FILE   = DATABASE_DIR / "trainer.bin"


def load_tokenizer():
    word_to_id = {}
    id_to_word = {}
    with open(TOKENIZER_FILE, "rb") as f:
        for raw in f:
            line = raw.decode("utf-8", errors="ignore").strip()
            sep  = line.find("|")
            if sep < 0:
                continue
            wid  = int(line[:sep])
            word = line[sep + 1:]
            word_to_id[word] = wid
            id_to_word[wid]  = word
    return word_to_id, id_to_word


def load_ngrams():
    ngrams = {}
    with open(TRAINER_FILE, "rb") as f:
        for raw in f:
            parts = raw.split(b"|")
            if len(parts) != 5:
                continue
            try:
                w1   = int(parts[1])
                w2   = int(parts[2])
                w3   = int(parts[3])
                freq = int(parts[4])
            except ValueError:
                continue
            key = (w1, w2)
            if key in ngrams:
                ngrams[key].append((w3, freq))
            else:
                ngrams[key] = [(w3, freq)]
    return ngrams


def generate_text(request: str, ngrams, word_to_id, id_to_word, bos_id=1, eos_id=2, max_tokens=50):
    if isinstance(request, list):
        request = " ".join(request)

    words = WORD_REGEX.findall(request.lower()) if request else []
    token_ids = [word_to_id[w] for w in words if w in word_to_id]

    if not token_ids:
        w1, w2 = bos_id, bos_id
    elif len(token_ids) == 1:
        w1, w2 = bos_id, token_ids[0]
    else:
        w1, w2 = token_ids[-2], token_ids[-1]

    result_ids = list(token_ids)

    for _ in range(max_tokens):
        rows = ngrams.get((w1, w2), [])
        if not rows:
            break

        next_id = random.choices([r[0] for r in rows], weights=[r[1] for r in rows])[0]
        if next_id == eos_id:
            break

        result_ids.append(next_id)
        w1, w2 = w2, next_id

    output_words = [id_to_word[tid] for tid in result_ids if tid in id_to_word]
    return " ".join(output_words)


def main():
    print("Chargement des données en cours...")
    print("- Chargement du vocabulaire...", flush=True)
    word_to_id, id_to_word = load_tokenizer()
    bos_id = word_to_id.get(TOKEN_START_SENTENCE, 1)
    eos_id = word_to_id.get(TOKEN_END_SENTENCE, 2)

    print("- Chargement des n-grams...", flush=True)
    ngrams = load_ngrams()
    print("Modèle prêt ! Tapez 'exit' pour quitter.\n")

    while True:
        try:
            req = input("IA > ").strip()
        except (KeyboardInterrupt, EOFError):
            print("\nSortie.")
            break

        if req.lower() in ("exit", "quit"):
            break

        if not req:
            continue

        output = generate_text(req, ngrams, word_to_id, id_to_word, bos_id, eos_id)
        print(output)
        print()


if __name__ == "__main__":
    main()
