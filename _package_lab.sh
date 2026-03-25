#!/bin/bash

read -rp "Numéro du lab (ex: 01) : " LAB_NUM

LAB_DIR="lab${LAB_NUM}"
ARCHIVE="urs_behrmann_lab${LAB_NUM}.tar.gz"

if [ ! -d "$LAB_DIR" ]; then
    echo "Erreur: le dossier '$LAB_DIR' n'existe pas."
    exit 1
fi

FILES=()

[ -d "$LAB_DIR/src" ]        && FILES+=("$LAB_DIR/src")
[ -d "$LAB_DIR/include" ]    && FILES+=("$LAB_DIR/include")
[ -f "$LAB_DIR/rapport.pdf" ] && FILES+=("$LAB_DIR/rapport.pdf")

if [ ${#FILES[@]} -eq 0 ]; then
    echo "Erreur: aucun fichier à archiver trouvé dans '$LAB_DIR'."
    exit 1
fi

tar -zcvf "$ARCHIVE" "${FILES[@]}"
echo "Archive créée : $ARCHIVE"
