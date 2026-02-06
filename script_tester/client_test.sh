#!/bin/bash

export CXX=g++
export CXXFLAGS="-Wall -Wextra -Werror -std=c++98 -O2 -g"
LOGS="www/logs"
IP="127.0.0.1"
PORT="1234"
HOST="http://$IP:$PORT"

make || exit 1

trap 'kill $SERVER_PID 2>/dev/null; kill $PARROT_PID 2>/dev/null' EXIT

# Lancement du serveur avec valgrind
valgrind --leak-check=full --track-fds=yes ./webserv config/server_test.json &> $LOGS/valgrind_server.log &
SERVER_PID=$!
echo "Server launched (PID $SERVER_PID) on $HOST"

# Attente du démarrage réel
timeout=5
while ! nc -z $IP $PORT && [ $timeout -gt 0 ]; do
	sleep 1
	((timeout--))
done

if [ $timeout -eq 0 ]; then
	echo "Échec du démarrage du serveur"
	kill $SERVER_PID
	exit 1
fi

# Compilation du client
$CXX $CXXFLAGS Client/ClientHttp.cpp Client/ClientResponse.cpp Client/ClientRequest.cpp Client/ClientTest.cpp -o ClientServer.out
if [ ! -x ClientServer.out ]; then
	echo "Erreur de compilation du client"
	exit 1
fi

# Lancement du test client avec valgrind
valgrind --track-fds=yes ./ClientServer.out > $LOGS/valgrind_client.log
result=$?



echo "🔻 Fermeture du serveur (PID $SERVER_PID)..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null


# Résumés
SERVER_SUMMARY=$(awk '/HEAP SUMMARY:/,0' "$LOGS/valgrind_server.log")
CLIENT_SUMMARY=$(awk '/HEAP SUMMARY:/,0' "$LOGS/valgrind_client.log")

# Contenus complets avec échappement HTML
SERVER_LOG=$(sed 's/&/&amp;/g; s/</\&lt;/g; s/>/\&gt;/g' "$LOGS/valgrind_server.log")
CLIENT_LOG=$(sed 's/&/&amp;/g; s/</\&lt;/g; s/>/\&gt;/g' "$LOGS/valgrind_client.log")

# Génération du rapport HTML
cat > $LOGS/valgrind_report.html <<EOF
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Rapports Valgrind</title>
  <!-- Highlight.js CSS thème sombre -->
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.11.1/styles/default.min.css">
  <style>
    body { font-family: sans-serif; margin: 2em; background: #f9f9f9; color: #222; }
    h1 { color: #444; }
    pre {
      padding: 1em;
      border-radius: 5px;
      overflow-x: auto;
      max-height: 600px;
    }
    a.button {
      display: inline-block;
      margin: 1em 0;
      padding: 0.5em 1em;
      background: #007acc;
      color: white;
      text-decoration: none;
      border-radius: 5px;
      font-weight: bold;
    }
    a.button:hover {
      background: #005f99;
    }
    details summary {
      cursor: pointer;
      margin-top: 1em;
      font-size: 1.1em;
    }
  </style>
</head>
<body>
  <h1>Rapports Valgrind</h1>

  <h2>🧠 Résumé Serveur</h2>
  <pre><code class="language-html">$(echo "$SERVER_SUMMARY" | sed 's/&/&amp;/g; s/</\&lt;/g; s/>/\&gt;/g')</code></pre>

  <h2>🧠 Résumé Client</h2>
  <pre><code class="language-html">$(echo "$CLIENT_SUMMARY" | sed 's/&/&amp;/g; s/</\&lt;/g; s/>/\&gt;/g')</code></pre>

  <details>
    <summary><strong>📝 Rapport Serveur complet</strong></summary>
    <pre><code class="language-html">$SERVER_LOG</code></pre>
  </details>

  <details>
    <summary><strong>📝 Rapport Client complet</strong></summary>
    <pre><code class="language-html">$CLIENT_LOG</code></pre>
  </details>

  <!-- Highlight.js JS -->

    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.11.1/highlight.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.11.1/languages/go.min.js"></script>
    <script>console.log("hello from firefox"); hljs.highlightAll();</script>
</body>
</html>
EOF

# ouverture des rapports valgrin dans une page html heberge par un mini serveur http pyton temporaire
pushd "$LOGS" > /dev/null
python3 -m http.server 8000 > /dev/null 2>&1 &
HTTP_SERVER_PID=$!
popd > /dev/null

REPORT_URL="http://127.0.0.1:8000/valgrind_report.html"

echo "🌐 Ouverture du rapport dans le navigateur..."
if command -v firefox &>/dev/null; then
	firefox --new-window "$REPORT_URL" > /dev/null 2>&1 &
	BROWSER_PID=$!
else
	xdg-open "$REPORT_URL"
	BROWSER_PID=$!  # note : xdg-open ne donne pas toujours un PID valide
fi

sleep 1

# Choix du perroquet
if [ "$result" -eq 42 ]; then
    gnome-terminal -- bash -c "./parrot.sh"
else
    gnome-terminal -- bash -c "./mad_parrot.sh"
fi

# Attente que le navigateur se ferme (optionnel : timeout max de sécurité)
echo "⏳ Attente de la fermeture du navigateur..."
wait $BROWSER_PID 2>/dev/null

# Arrêt du serveur HTTP local
echo "🛑 Arrêt du mini serveur HTTP local (PID $HTTP_SERVER_PID)..."
kill $HTTP_SERVER_PID 2>/dev/null
wait $HTTP_SERVER_PID 2>/dev/null
