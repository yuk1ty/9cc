#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 2 "-3+5;"
try 1 "42==42;"
try 1 "42!=21;"
try 1 "0<1;"
try 1 "1>0;"
try 1 "1>=0;"
try 1 "0<=1;"
try 1 "42>=42;"
try 1 "42<=42;"
try 3 "a = 1; b = 2; c = a + b; c;"
try 2 "a = 1; b = 2; c = a * b; c;"
try 9 "a = 1; b = 2; c = 3; d = (a + b) * 3; d;"
try 1 "a = 42; b = 21; c = a > b; c;"

echo OK