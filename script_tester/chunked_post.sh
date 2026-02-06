#!/bin/bash

# Vérification des arguments
if [ $# -ne 1 ]; then
  echo "Usage: $0 <file>"
  exit 1
fi

FILE="$1"
URL="http://127.0.0.1:8080/point.bla"
URL="http://127.42.43.44:1234/point.bla"

# Vérifie que le fichier existe
if [ ! -f "$FILE" ]; then
  echo "Fichier non trouvé : $FILE"
  exit 2
fi

CXX=g++
CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -O3 -g"

make -s || exit 1

trap 'kill $SERVER_PID 2>/dev/null' EXIT

# valgrind --leak-check=full --track-fds=yes ./webserv config/server_test.json &
valgrind ./webserv config/server_test.json &

SERVER_PID=$!
echo "Server launched (PID $SERVER_PID) listen on $URL"

sleep 1


#curl -v http://localhost:8080/download -X PUT --header "Transfer-Encoding: chunked" --header "Filename: test.txt" --header "Content-Type: application/octet-stream" --data-binary @"www/into_www/welcome.py" --http1.1


# Utilisation de curl avec --chunked pour forcer Transfer-Encoding: chunked
curl -v "$URL" \
  -X POST \
  --header "Transfer-Encoding: chunked" \
  --header "Filename: $FILE" \
  --header "Content-Type: application/octet-stream" \
  --data-binary @"$FILE" \
  --http1.1

echo "Arrêt du serveur (PID $SERVER_PID)..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
# curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
# curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
# curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
#   curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
#   curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
#   curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1
#   curl -v "$URL" \
#   -X POST \
#   --header "Transfer-Encoding: chunked" \
#   --header "Content-Type: application/octet-stream" \
#   --data-binary @"$FILE" \
#   --http1.1

if [ $? -eq  0 ];then
xterm -e ./parrot.sh &
else
xterm -e ./mad_parrot.sh &
fi

# echo "Arrêt du serveur (PID $SERVER_PID)..."
# kill $SERVER_PID
# wait $SERVER_PID 2>/dev/null
