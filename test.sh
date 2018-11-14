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
        echo "$expected expected, but got $actual in expression \"$2\""
        exit 1
    fi

    set -e
}

try 0 0
try 42 42
try 42 "21+23-2"
try 42 "21-2+23"
try 42 "  21  +  23  -  2   "
try 47 "5+6*7"
try 15 "5*(9-6)"
try 4 "(3+5)/2"
try 26 "(3 - 1)*(9-6) + (7-2)*(2-1+3)"

echo "OK"