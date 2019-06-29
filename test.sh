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

# if statement

try 1 "x = 9; if ( x == 9 ) return 1; return 0;"
try 2 "x = 11; if ( x == 9 ) return 1; return 2;"
try 2 "x = 11; if ( x == 9 ) return 1; else return 2; return 3;"
try 1 "x = 10; if ( x == 10 ) return 1; else if(x == 11) return 2; else return 3; return 4;"
try 2 "x = 11; if ( x == 10 ) return 1; else if(x == 11) return 2; else return 3; return 4;"
try 3 "x = 12; if ( x == 10 ) return 1; else if(x == 11) return 2; else return 3; return 4;"
try 3 "x = 13; if ( x == 10 ) return 1; else if(x == 11) return 2; else return 3; return 4;"

# math expression

try 0 "0;"
try 42 "42;"
try 42 "21+23-2;"
try 42 "21-2+23;"
try 42 "  21  +  23  -  2  ; "
try 47 "5+6*7;   "
try 15 "5*(9-6)   ;"
try 4 "(3+5)/2;"
try 26 "(3 - 1)*(9-6) + (7-2)*(2-1+3);"
try 253 "-3;" # because 11111101 = 253
try 4 "+4;"
try 2 "3 + -1;"
try 4 "1 - -3;"
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

# test comparator
try 1 "11 == 11;"
try 0 "11 == 10;"

try 0 "11 != 11;"
try 1 "11 != 10;"

try 1 "11 <= 11;"
try 1 "10 <= 11;"
try 0 "10 <= 9;"

try 0 "11 < 11;"
try 1 "10 < 11;"
try 0 "10 < 9;"

try 1 "11 >= 11;"
try 0 "10 >= 11;"
try 1 "10 >= 9;"

try 0 "11 > 11;"
try 0 "10 > 11;"
try 1 "10 > 9;"

# test associative
try 0 "1 == (2 <= 1);"
try 1 "(1 == 2) <= 1;"
try 0 "1 == 2 <= 1;"

try 0 "1 != (0 <= 1);"
try 1 "(1 != 0) <= 1;"
try 0 "1 != 0 <= 1;"

# test long statement
try 4 "a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=2;a=a+a;a;"

# simple stetement
try 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
try 9 "b = a = 3; a * b;"

# long varname
try 6 \
" foo = 1; \
  bar = 2 + 3;\
  foo + bar;"
try 10 "n1 = 1; n2 = 2; n3 = 3; n4 = 4; n1+n2+n3+n4;"

# return
try 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"
try 5 "return 5; return 8;"


echo "OK"