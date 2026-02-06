<?php
// Petit script de test PHP

// Définir le Content-Type
header('Content-Type: text/plain; charset=utf-8');
// Définir la Date au format HTTP/1.1
header('Date: ' . gmdate('D, d M Y H:i:s') . ' GMT');

echo "=== Test index.php PHP CGI ===\n\n";

// Paramètres GET
if (!empty($_GET)) {
    echo "Paramètres GET :\n";
    foreach ($_GET as $key => $value) {
        echo "  $key = $value\n";
    }
    echo "\n";
}

// Paramètres POST
if (!empty($_POST)) {
    echo "Paramètres POST :\n";
    foreach ($_POST as $key => $value) {
        echo "  $key = $value\n";
    }
    echo "\n";
}

// Body brut (par exemple JSON)
echo "Corps brut de la requête :\n";
$rawBody = file_get_contents('php://input');
if ($rawBody !== '') {
    echo $rawBody . ": received\n\n";
}

// Variables d'environnement CGI
echo "Variables d'environnement CGI :\n";
foreach ($_SERVER as $key => $value)
{
    echo "  $key = $value\n";
}

echo "=== FIN du test PHP CGI ===\n\n";

?>