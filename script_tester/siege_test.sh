#!/bin/bash

# Vérification des arguments
if [ $# -ne 2 ]; then
  echo "Usage: $0 <concurrency> <duration in second>"
  exit 1
fi

CXX=g++
CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -O3"
HOSTNAME="http://127.42.43.44:1234"
CONCURRENCY=$1
DURATION=$2
TIME="${DURATION}S"

# Compilation
make -s || exit 1

trap 'kill $SERVER_PID 2>/dev/null' EXIT

# Lancer le serveur
./webserv config/server_test.json 1>/dev/null &
SERVER_PID=$!
echo "Server launched (PID $SERVER_PID) listen on $HOSTNAME"
sleep 1

# === Barre d'avancement ===
function progress_bar() {
    local duration=$1
    local width=30
    local interval=0.2
    local steps=$(echo "$duration / $interval" | bc)
    local increment=$(echo "$width / $steps" | bc -l)

    local GREEN="\033[1;32m"
    local RESET="\033[0m"

    for ((i = 0; i < steps; i++)); do
        filled=$(echo "$i * $increment" | bc)
        filled=${filled%.*}
        empty=$((width - filled))

        bar=$(printf "%0.s#" $(seq 1 "$filled"))
        bar+=$(printf "%0.s_" $(seq 1 $empty))
        percent=$(( (100 * i) / steps ))

        # effacer ligne + curseur début + afficher la barre
        printf "\r\033[KProgress: ${GREEN}[%s] %3d%%%s" "$bar" "$percent"
        sleep $interval
    done

    printf "\r\033[KProgress: ${GREEN}[%s] 100%%%s\n" "$(printf "%0.s#" $(seq 1 $width))"
    echo -e "$RESET"
}

# Lancer barre de progression
echo "Siege launched with $CONCURRENCY concurrency for $TIME"
progress_bar "$DURATION" &
BAR_PID=$!

# Lancer siege mais capter la sortie temporairement pour l'afficher proprement après
TMP_OUT=$(mktemp)
siege -b -c "$CONCURRENCY" -t"$TIME" "$HOSTNAME" > "$TMP_OUT" 2>&1
wait $BAR_PID 2>/dev/null

# Afficher sortie de siege après la barre proprement
echo ""
cat "$TMP_OUT"
rm -f "$TMP_OUT"

# Tuer le serveur
echo "Arrêt du serveur (PID $SERVER_PID)..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
