#!/bin/bash
set -e

PORT=6380
WAL_FILE="wal_test.log"
BINARY="./build/redis-lite"

echo "=== Building redis-lite ==="
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..

echo "=== Starting server on port $PORT ==="
$BINARY -p $PORT -w $WAL_FILE -t 2 &
SERVER_PID=$!
sleep 1

cleanup() {
    echo "=== Stopping server ==="
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    rm -f $WAL_FILE
}
trap cleanup EXIT

send_cmd() {
    echo -ne "$1\r\n" | nc -w1 127.0.0.1 $PORT
}

echo "=== Test 1: PING ==="
RESP=$(send_cmd "*1\r\n\$4\r\nPING")
if [[ "$RESP" == "+PONG"* ]]; then
    echo "PASS"
else
    echo "FAIL: Expected +PONG, got '$RESP'"
    exit 1
fi

echo "=== Test 2: SET and GET ==="
send_cmd "*3\r\n\$3\r\nSET\r\n\$3\r\nfoo\r\n\$3\r\nbar"
RESP=$(send_cmd "*2\r\n\$3\r\nGET\r\n\$3\r\nfoo")
if [[ "$RESP" == *"bar"* ]]; then
    echo "PASS"
else
    echo "FAIL: Expected bar, got '$RESP'"
    exit 1
fi

echo "=== Test 3: DEL ==="
RESP=$(send_cmd "*2\r\n\$3\r\nDEL\r\n\$3\r\nfoo")
if [[ "$RESP" == ":1"* ]]; then
    echo "PASS"
else
    echo "FAIL: Expected :1, got '$RESP'"
    exit 1
fi

echo "=== Test 4: GET after DEL ==="
RESP=$(send_cmd "*2\r\n\$3\r\nGET\r\n\$3\r\nfoo")
if [[ "$RESP" == *"$-1"* ]]; then
    echo "PASS"
else
    echo "FAIL: Expected $-1 (null), got '$RESP'"
    exit 1
fi

echo "=== Test 5: WAL recovery ==="
send_cmd "*3\r\n\$3\r\nSET\r\n\$4\r\npersist\r\n\$4\r\ntest"
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null || true

$BINARY -p $PORT -w $WAL_FILE -t 2 &
SERVER_PID=$!
sleep 1

RESP=$(send_cmd "*2\r\n\$3\r\nGET\r\n\$7\r\npersist")
if [[ "$RESP" == *"test"* ]]; then
    echo "PASS"
else
    echo "FAIL: WAL recovery did not restore 'persist' key, got '$RESP'"
    exit 1
fi

echo "=== All tests passed ==="
exit 0