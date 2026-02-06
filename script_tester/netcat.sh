#!/bin/bash

host=localhost
port=9999
errors=0
counter=0

head -c 80000000 < /dev/zero > /tmp/test_max

run_large_upload()
{
  local name="$1"
  local filename="$2"
  local file="$3"
  local exp="$4"

  echo -e "\e[34m==== Running test: $name ====\e[0m"
  echo -e "\e[34mRequest hexdump (first 128 bytes):\e[0m"
  (
    printf "PUT /download HTTP/1.1\r\n"
    printf "Host: %s\r\n" "$host"
    printf "Content-Type: application/octet-stream\r\n"
    printf "X-Filename: %s\r\n" "$filename"
    printf "Content-Length: %s\r\n" "$(stat -c%s "$file")"
    printf "\r\n"
    head -c 128 "$file"
  ) | hexdump -C

  # Stream the full upload
  local response
  response=$(
    (
      printf "PUT /download HTTP/1.1\r\n"
      printf "Host: %s\r\n" "$host"
      printf "Content-Type: application/octet-stream\r\n"
      printf "X-Filename: %s\r\n" "$filename"
      printf "Content-Length: %s\r\n" "$(stat -c%s "$file")"
      printf "\r\n"
      cat "$file"
    ) | nc -w 5 "$host" "$port" || true
  )

  if grep -Fq "$exp" <<<"$response"; then
    echo -e "\e[32mPASS: found expected -> $exp\e[0m"
    return 0
  else
    echo -e "\e[31mFAIL: expected -> $exp"
    echo "Full response:"
    echo -e "$response\e[0m"
    return 1
  fi
}


run_command()
{
  local name="$1"
  local req="$2"
  local exp="$3"

  echo -e "\e[34m==== Running test: $name ====\e[0m"
  echo -e "\e[34mRequest hexdump:\e[0m"
  echo "$req" | hexdump -C

  local response=$(echo -ne "$req" | nc -w 2 "$host" "$port" || true)

  if grep -Fq "$exp" <<<"$response"; then
    echo -e "\e[32mPASS: found expected -> $exp\e[0m"
	return 0
  else
    echo -e "\e[31mFAIL: expected -> $exp"
    echo "Full response:"
    echo -e "$response\e[0m"
	return 1
  fi
  echo
}

US=$'\x1F'   # unit separator
RS=$'\x1E'   # record separator

tests=(
$'00 Root test'"${RS}"'GET / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'01 Not Found'"${RS}"'GET /notfound HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'02 Bad Request'"${RS}"'GET /notfound HTTP/1.\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'03 URI Too Long'"${RS}"'GET /'"$(head -c 5000 < /dev/zero | tr '\0' 'A')"$' HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'414 Uri Too Long'
$'04 Header Too Large'"${RS}"'GET / HTTP/1.1\r\nHost: '"$(head -c 5000 < /dev/zero | tr '\0' 'B')"$'\r\n\r\n'"${US}"$'431 Request Header Fields Too Large'
$'05 Directory Traversal'"${RS}"'GET /../ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'06 GET Static File'"${RS}"'GET /index.html HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'07 HEAD Static File'"${RS}"'HEAD /index.html HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'08 GET font.css'"${RS}"'GET /font.css HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'09 GET .env'"${RS}"'GET /.env HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'403 Forbidden'
$'10 HEAD .env'"${RS}"'HEAD /.env HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'403 Forbidden'
$'11 GE Wrong First Line'"${RS}"'GE / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'12 Wrong First Line'"${RS}"'/ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'13 Wrong First Line'"${RS}"'GET HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'14 Wrong First Line'"${RS}"'GET / HTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'15 Wrong First Line'"${RS}"'GET / HTTP/11\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'16 Wrong First Line'"${RS}"'GET / HTTP/1.1\r\nHot: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'17 Wrong First Line'"${RS}"'GET / HTTP/1.1\r\nHo t: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'18 Wrong First Line'"${RS}"'GET / HTTP/1.1\r\nHost:\r\n\r\n'"${US}"$'400 Bad Request'
$'19 Wrong First Line'"${RS}"'GET / HTTP/1.1\r\nHost: \r\n\r\n'"${US}"$'400 Bad Request'
$'20 Wrong First Line'"${RS}"'GET / HTTP/1.1\r\nHost:\n\r\n\r\n'"${US}"$'400 Bad Request'
$'21 Empty Request Line'"${RS}"'\r\n\r\n'"${US}"$'400 Bad Request'
$'22 GET Only \\n'"${RS}"'GET / HTTP/1.1\nHost: nc\n\n'"${US}"$'400 Bad Request'
$'23 No Host Header'"${RS}"'GET / HTTP/1.1\r\n\r\n'"${US}"$'400 Bad Request'
$'24 Duplicated Headers'"${RS}"'GET / HTTP/1.1\r\nHost: nc\r\nHost: duplicate\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'25 Mixed case headers'"${RS}"'GET / HTTP/1.1\r\nhOsT: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'26 Whitepaces in first line'"${RS}"'GET   /   HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'27 Invalid HTTP Version'"${RS}"'GET / HTTP/9.9\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'28 Multiple requests in one connection'"${RS}"'GET / HTTP/1.1\r\nHost: nc\r\n\r\nGET /notfound HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK\n404 Not Found'
$'29 GET /www Directory listing'"${RS}"'GET /www HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'30 GET /www/load_component.js'"${RS}"'GET /www/load_components.js HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'31 GET /directory'"${RS}"'GET /directory HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'200 OK'
$'32 POST /directory/*.bla'"${RS}"'POST /directory/test.bla HTTP/1.1\r\nHost: nc\r\nContent-Length:4\r\n\r\nBODY\r\n'"${US}"$'HTTP/1.1 200 OK'
$'33 GET /php/*.php'"${RS}"'GET /php/echo.php HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'34 POST /php/*.php'"${RS}"'POST /php/echo.php HTTP/1.1\r\nHost: nc\r\nContent-Length:6\r\n\r\nFOOBAR\r\n'"${US}"$'HTTP/1.1 200 OK'
$'35 POST /post_body'"${RS}"'POST /post_body HTTP/1.1\r\nHost: nc\r\nContent-Length:5\r\n\r\nHELLO\r\n'"${US}"$'HTTP/1.1 200 OK'
$'36 HEAD /download/*'"${RS}"'HEAD /download/img.png HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'38 GET /admin/'"${RS}"'GET /admin/ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'39 GET /admin/*'"${RS}"'GET /admin/somefile.txt HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'40 OPTIONS /'"${RS}"'OPTIONS / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'501 Not Implemented'
$'41 TRACE /'"${RS}"'TRACE / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'501 Not Implemented'
$'42 POST /www/load_components.js'"${RS}"'POST /www/load_components.js HTTP/1.1\r\nHost: nc\r\nContent-Length:4\r\n\r\nTEST\r\n'"${US}"$'405 Method Not Allowed'
$'43 GET /directory/foo.bla'"${RS}"'GET /directory/foo.bla HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'45 HEAD /directory/foo.bla'"${RS}"'HEAD /directory/foo.bla HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'46 POST Long Body /post_body'"${RS}"'POST /post_body HTTP/1.1\r\nHost: nc\r\nContent-Length:101\r\n\r\n'"$(head -c 101 < /dev/zero | tr '\0' 'A')"$'\r\n'"${US}"$'413 Content Too Large'
$'47 POST Long Body /php/*.php'"${RS}"'POST /php/echo.php HTTP/1.1\r\nHost: nc\r\nContent-Length:1001\r\n\r\n'"$(head -c 1001 < /dev/zero | tr '\0' 'B')"$'\r\n'"${US}"$'413 Content Too Large'
$'48 GET Nested traversal'"${RS}"'GET /directory/../../etc/passwd HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'49 GET encoded URL'"${RS}"'GET /%2e%2e/%2e%2e/%2e%2e/etc/passwd HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'50 GET /directory/'"${RS}"'GET /directory/ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'51 POST Weird Content'"${RS}"'POST /directory/test.bla HTTP/1.1\r\nHost: nc\r\nContent-Length:14\r\n\r\nHELLO%00WORLD\r\n'"${US}"$'HTTP/1.1 200 OK'
$'52 UPLOAD file'"${RS}"'PUT /download HTTP/1.1\r\nHost: nc\r\nContent-Type: application/octet-stream\r\nX-Filename: testfile\r\nContent-Length: 11\r\n\r\nHELLO WORLD'"${US}"$'HTTP/1.1 200 OK'
$'53a GET Uploaded file'"${RS}"'GET /download/testfile HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'53b HEAD Uploaded file'"${RS}"'HEAD /download/testfile HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'55 Malformed Header'"${RS}"'GET / HTTP/1.1\r\nHost nc\r\n\r\n'"${US}"$'400 Bad Request'
$'56a Extra CR in headers'"${RS}"'GET / HTTP/1.1\r\nHost: nc\r\r\n\r\n'"${US}"$'400 Bad Request'
$'56b Extra CR in headers'"${RS}"'GET / HTTP/1.1\r\nHost: \rnc\r\n\r\n'"${US}"$'400 Bad Request'
$'56c Extra LF in headers'"${RS}"'GET / HTTP/1.1\r\nHost: \nnetcat\r\n\r\n'"${US}"$'400 Bad Request'
$'57 Lowercase method'"${RS}"'get / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'400 Bad Request'
$'58  CONNECT method'"${RS}"'CONNECT / HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'501 Not Implemented'
$'59 DELETE /download/testfile'"${RS}"'DELETE /download/testfile HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'405 Method Not Allowed'
$'60 OPTIONS with body'"${RS}"'OPTIONS / HTTP/1.1\r\nHost: nc\r\nContent-Length:4\r\n\r\nTEST\r\n'"${US}"$'501 Not Implemented'
$'61 POST Chunked'"${RS}"'POST /post_body HTTP/1.1\r\nHost: nc\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHELLO\r\n0\r\n\r\n'"${US}"$'200 OK'
$'62 GET file with spaces'"${RS}"'GET /www/file%20with%20spaces.txt HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'63 GET hidden file'"${RS}"'GET /www/.hidden HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'404 Not Found'
$'64 Max Body at limit'"${RS}"'POST /php/echo.php HTTP/1.1\r\nHost: nc\r\nContent-Length:1000\r\n\r\n'"$(head -c 1000 < /dev/zero | tr '\0' 'B')"$'\r\n'"${US}"$'HTTP/1.1 200 OK'
$'65 Max Body over limit'"${RS}"'POST /php/echo.php HTTP/1.1\r\nHost: nc\r\nContent-Length:1002\r\n\r\n'"$(head -c 1002 < /dev/zero | tr '\0' 'C')"$'\r\n'"${US}"$'413 Content Too Large'
$'66 GET directory no listing'"${RS}"'GET /directory HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'200 OK'
$'67 GET directory index'"${RS}"'GET /www/ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'68 POST empty body'"${RS}"'POST /post_body HTTP/1.1\r\nHost: nc\r\nContent-Length:0\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'69 GET with query'"${RS}"'GET /index.html?user=test HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'HTTP/1.1 200 OK'
$'70 GET invalid percent'"${RS}"'GET /%ZZ HTTP/1.1\r\nHost: nc\r\n\r\n'"${US}"$'200 OK'
)

for t in "${tests[@]}"; do
  name="${t%%"$RS"*}"
  rest="${t#*"$RS"}"
  req="${rest%%"$US"*}"
  exp="${rest#*"$US"}"

  if ! run_command "$num $name" "$req" "$exp"; then
    errors=$((errors + 1))
  fi
  counter=$((counter + 1))
done

# run_large_upload "54 UPLOAD big file" "test_max" "/tmp/test_max" "HTTP/1.1 413 Content Too Large"

echo
echo "===================================="
echo -e "\e[34mTotal tests: $counter"
echo -e "\e[31mErrors:      $errors"
echo -e "\e[32mPassed:      $((counter - errors))\e[0m"
echo "===================================="


if [ "$errors" -eq 0 ]; then
  echo -e "\e[32mCongratulations! All tests passed!\e[0m"
else
  echo -e "\e[31mSome tests didn't pass.\e[0m"
fi

rm -rf /goinfre/download/testfile
rm -rf /tmp/test_max
