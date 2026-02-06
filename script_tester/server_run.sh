#!/bin/bash

# Vérification des arguments
if [ $# -ne 3 ]; then
  echo "Usage: $0 <executable> <hostname> <port>"
  exit 1
fi

make Makefile_ja || exit 1

trap 'kill $SERVER_PID 2>/dev/null; kill $PARROT_PID 2>/dev/null' EXIT

valgrind  --leak-check=full --track-fds=yes ./$1 config/config.json &

SERVER_PID=$!
# Attente du démarrage réel
timeout=5
while ! nc -z "$2" "$3" && [ $timeout -gt 0 ]; do
	sleep 1
	((timeout--))
done

if [ $timeout -eq 0 ]; then
	echo "Échec du démarrage du serveur"
	kill $SERVER_PID
	exit 1
fi
echo "WEBSERV launched (PID $SERVER_PID)"
firefox --private-window "http://$2:$3" > /dev/null 2>&1 &
BROWSER_PID=$!

echo "⏳ Attente de la fermeture du navigateur..."
wait $BROWSER_PID 2>/dev/null

echo "🔻 Fermeture du serveur (PID $SERVER_PID)..."
kill $SERVER_PID
wait $SERVER_PID
