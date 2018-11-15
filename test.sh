#!/bin/bash

set -eu

try(){
    set +e

    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s

    if [[ $? != 0 ]]; then
        echo "unsuccess compilation: $input"
        exit 1
    fi

    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [[ "$actual" != "$expected" ]]; then
        echo "$expected expected, but got $actual in expression \"$2\""
        exit 1
    fi

    set -e
}

try 0 "0;"
try 42 "42;"
try 42 "21+23-2;"
try 42 "21-2+23;"
try 42 "  21  +  23  -  2  ; "
try 47 "5+6*7;   "
try 15 "5*(9-6)   ;"
try 4 "(3+5)/2;"
try 26 "(3 - 1)*(9-6) + (7-2)*(2-1+3);"
try 1 "a=1;"
try 2 "a=1;b=2;"
try 3 "a=1;b=2;b=3;"
try 3 "z=1;y=2;x=3;"
try 100 "a=100;a;"
try 0 "a=1;b=2;c=a+b;d=c-c;z=d;"
try 1 "a=1; b=1; a = a == b; a;"
try 0 "a=1; b=1; a = a != b; a;"
try 1 "a=1; b=2; c = a != b; c;"
try 1 "a = (2 + 3 == 5); a;"
try 2 "a = (2 + (3 == 5)); a;"
try 1 "a = 2 == 2; a;"
try 1 "a = (2 == 2); a;"
# try 1 "(a = 2) == 2; a;"

echo "OK"