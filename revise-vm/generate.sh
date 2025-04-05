#!/usr/bin/env bash

if [ $# -lt 5 ]; then
  >&2 echo "Usage: generate.sh NUM_VARS NUM_VALS NUM_CONSTRAINTS TIGHTNESS NUM_CONSTRAINT_DEFS [I]"
  exit 1
fi
n="$1" # num vars
d="$2" # num vals
c="$3" # num constraints
t="$4" # tightness (num no goods)
k="$5" # num constriant definitions
i='0'
if [ $# -ge 6 ]; then i=$6; fi

>&2 echo "n${n}d${d}c${c}t${t}i${i}k${k}"

for s in $(seq 100); do
  cj-gen-urbcsp $n $d $c $t $s 0 $k > t.json;
  name=$(jq -r '.meta.id' t.json);
  file="instances/vars$n-vals$d-defs$k/$name.json"
  mkdir -p $(dirname "$file")
  mv t.json $file
done
