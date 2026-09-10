# C IA Builder

CIAB (C IA Builder) est un projet opensource qui a pour but de fournir des fonctions écrites en C pour créer des modèles d'IA qui génère du texte de manière économique et très basique.

# Comment ça marche ?

## Datasets

Les datasets sont un ensemble de donné utilisé pour entrainer et fournir les connaissances au modèle d'IA

## Préparation du modèle

Le programme "creator" va commencer par lire tout les fichiers et récupérer tout les mots existants uniques dans les datasets fournis, ensuite on va chercher la probabilité que un mot X apparaît après 2 mot Y et Z.

## Stockage

Le modèle sera sauvegardé dans un fichier .db en utilisant sqlite comme système de base de donnée dans un premier temps.

## Génération

Le programme "generator" va lire le fichier .db passé en argument et va ouvrir une interface CLI pour que vous fassiez les requêtes au modèle.