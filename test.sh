#!/bin/bash

set -eu

try(){
    set +e

    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [[ "$actual" != "$expected" ]]; then
        echo "$expected expected, but got $actual"
        exit 1
    fi

    set -e
}

try 0 0
try 42 42
try 42 "21+23-2"
try 42 "  21  +  23  -  2   "

echo "OK"