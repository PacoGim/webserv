# curl http://localhost:8080

# curl --header "Content-Type: application/json" \
#   --request POST \
#   http://localhost:8080

curl --header "Content-Type: application/json" \
  --request POST \
  --data '{"username":"xyz","password":"xyz"}' \
  http://localhost:8080

# curl http://localhost:8080


# (
#   echo -n "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test"
#   sleep 1
#   echo -n "\r\nAccept: */*\r\n\r\n"
# ) | nc localhost 8080 &

# (
#   echo -n "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test"
#   sleep 1
#   echo -n "\r\nAccept: */*\r\n\r\n"
# ) | nc localhost 8080 &

# (
#   echo -n "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test"
#   sleep 1
#   echo -n "\r\nAccept: */*\r\n\r\n"
# ) | nc localhost 8080 &

# (
#   echo -n "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test"
#   sleep 1
#   echo -n "\r\nAccept: */*\r\n\r\n"
# ) | nc localhost 8080 &

# (
#   echo -n "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test"
#   sleep 1
#   echo -n "\r\nAccept: */*\r\n\r\n"
# ) | nc localhost 8080
