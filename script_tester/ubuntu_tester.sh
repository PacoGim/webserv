#!/bin/bash

BIN=webserv
HOST=localhost
PORT=8080

CXXFLAGS=-O3 make || exit 1

trap 'kill $SERVER_PID 2>/dev/null' EXIT

./$BIN &
SERVER_PID=$!

# Attendre serveur ready
timeout=5
while ! nc -z $HOST $PORT && [ $timeout -gt 0 ]; do
	sleep 1
	((timeout--))
done

if [ $timeout -eq 0 ]; then
	echo "❌ Échec du démarrage du serveur"
	kill $SERVER_PID
	exit 1
fi

echo "✅ Serveur lancé (PID $SERVER_PID)"

SECONDS=0
printf '\n\n\n\n\n' | ./ubuntu_tester "http://$HOST:$PORT"
echo "⏱️ Durée totale des tests (wall time) : ${SECONDS} secondes"

echo ""
echo "🔻 Fermeture du serveur (PID $SERVER_PID)..."
# Arrêter le serveur
kill $SERVER_PID
wait $SERVER_PID
