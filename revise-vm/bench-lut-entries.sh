#!/usr/bin/env bash

exe=(./cj-solve-csp01)

for e in 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
  for i in $(seq 3); do
    ./bench2.sh ../csp-json-archive/data/urbcsp/n16d64c98t2048s26i0k10 "${exe[*]} -e $e"
    ./bench2.sh ../csp-json-archive/data/urbcsp/n16d128c96t9338s87i0k10 "${exe[*]} -e $e"
    ./bench2.sh ../csp-json-archive/data/urbcsp/n12d256c52t47382s57i0k10 "${exe[*]} -e $e"
    ./bench2.sh ../csp-json-archive/data/urbcsp/n10d512c36t217579s42i1k5 "${exe[*]} -e $e"
    ./bench2.sh ../csp-json-archive/data/urbcsp/n10d1024c36t891289s30i1k5 "${exe[*]} -e $e"
  done
done

