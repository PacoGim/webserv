#!/bin/bash

# HOST="localhost"
HOST=$(hostname)
PORT=8080

# Délai entre les morceaux (en secondes)
DELAY=1s

# Commande nc avec timeout de 10s en lecture
exec 3<>/dev/tcp/$HOST/$PORT

# Fonction pour envoyer un morceau avec délai
send_piece() {
    local piece="$1"
    printf "%b" "$piece" >&3
    sleep $DELAY
}

# Exemple : envoyer une requête GET par petits morceaux
send_piece "GET / HTTP/1"
send_piece ".1\r\nHost: $HOST\r\nUser"
send_piece "-Agent: HachureBot/1."

send_piece "0\r\nConnection: close\r\nAccept: */"
# Fin des headers (ligne vide)
send_piece "*\r\n\r\n"

# Lire la réponse (optionnel)
sleep 10
cat <&3
sleep 10

# Fermer la connexion
exec 3>&-
