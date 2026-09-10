import os
import re
import sys
import argparse
from typing import Optional
from datasets import load_dataset

try:
    from tqdm import tqdm
except ImportError:
    tqdm = None


def sanitize_filename(title: str, max_length: int = 150) -> str:
    clean = re.sub(r'[<>:"/\\|?*\x00-\x1f]', '_', title)
    clean = clean.strip('. ')
    if not clean:
        clean = "sans_titre"

    name_base = clean.split('.')[0].upper()
    reserved = {"CON", "PRN", "AUX", "NUL", *(f"COM{i}" for i in range(1, 10)), *(f"LPT{i}" for i in range(1, 10))}
    if name_base in reserved:
        clean = f"_{clean}"

    if len(clean) > max_length:
        clean = clean[:max_length].rstrip('. ')

    return clean


def crawl_wikipedia(
    output_dir: str = "../datasets/wikipedia",
    lang: str = "fr",
    date: str = "20231101",
    limit: Optional[int] = None,
):
    os.makedirs(output_dir, exist_ok=True)
    dataset_name = f"{date}.{lang}"
    print(f"Chargement du dataset wikimedia/wikipedia ({dataset_name}) en mode streaming...")
    dataset = load_dataset("wikimedia/wikipedia", dataset_name, streaming=True)

    print(f"Dossier de destination : {os.path.abspath(output_dir)}")
    if limit:
        print(f"Limite : {limit} articles.")
    else:
        print("Limite : aucune (appuyez sur Ctrl+C pour interrompre à tout moment).")

    count = 0
    skipped = 0

    pbar = tqdm(total=limit, desc="Téléchargement des articles", unit="art") if tqdm is not None else None

    try:
        for article in dataset["train"]:
            title = article.get("title", "").strip()
            text = article.get("text", "").strip()

            if not title or not text:
                continue

            safe_name = sanitize_filename(title)
            filepath = os.path.join(output_dir, f"{safe_name}.txt")

            if os.path.exists(filepath):
                skipped += 1
                continue

            with open(filepath, "w", encoding="utf-8") as f:
                f.write(text)

            count += 1
            if pbar is not None:
                pbar.update(1)
            elif count % 100 == 0:
                print(f"{count} articles sauvegardés...")

            if limit is not None and count >= limit:
                break

    except KeyboardInterrupt:
        print("\n\nTéléchargement interrompu par l'utilisateur.")
    finally:
        if pbar is not None:
            pbar.close()

    print(f"\nTerminé ! {count} articles enregistrés dans '{output_dir}' ({skipped} ignorés car déjà existants).")


def main():
    parser = argparse.ArgumentParser(
        description="Télécharge des articles Wikipédia sous forme de fichiers .txt dans datasets/wikipedia"
    )
    parser.add_argument(
        "--output-dir",
        "-o",
        default="datasets/wikipedia",
        help="Dossier où sauvegarder les fichiers .txt (défaut: datasets/wikipedia)",
    )
    parser.add_argument(
        "--lang",
        "-l",
        default="fr",
        help="Code de langue Wikipédia (défaut: fr)",
    )
    parser.add_argument(
        "--date",
        "-d",
        default="20231101",
        help="Date du dump Wikipédia (défaut: 20231101)",
    )
    parser.add_argument(
        "--limit",
        "-n",
        type=int,
        default=None,
        help="Nombre maximal d'articles à télécharger (défaut: illimité)",
    )

    args = parser.parse_args()
    crawl_wikipedia(
        output_dir=args.output_dir,
        lang=args.lang,
        date=args.date,
        limit=args.limit,
    )


if __name__ == "__main__":
    main()