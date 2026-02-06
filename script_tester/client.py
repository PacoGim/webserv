#!/usr/bin/python3
import socket
import time

# Création du socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connexion au serveur
s.connect(('localhost', 8080))

# Envoi d'une requête HTTP minimale (GET / HTTP/1.1)
# s.sendall(b"ihsdiof")
s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
# Réception de la réponse
response = s.recv(2048)
print(response.decode())
time.sleep(12)
response = s.recv(2048)
print(response.decode())
# Fermeture du socket
s.close()
