#!/usr/bin/env bash

# d64 is special - neon version does not work.
exed=(./cj-solve-csp01 ./cj-solve-csp01-1bit)
exe=(./cj-solve-csp01 pycsp3-wrapper.sh)

files=()
d='instances/vars16-vals64-defs10/urbcsp'
files+=("$d/n16d64c98t2048s7i0k10" "$d/n16d64c98t2048s2i0k10" "$d/n16d64c98t2048s99i0k10" "$d/n16d64c98t2048s79i0k10" "$d/n16d64c98t2048s56i0k10")

for i in $(seq 3); do
  for f in ${files[@]}; do
    ./bench.sh "$f" "${exed[*]/%/-d512}" ${exe[*]} ${exe[*]}
  done
done

# all other domain sizes are ok.
exed=(./cj-solve-csp01-neon ./cj-solve-csp01 ./cj-solve-csp01-1bit)
exe=(./cj-solve-csp01 pycsp3-wrapper.sh)

d='instances/vars16-vals128-defs10/urbcsp'
files+=("$d/n16d128c96t9338s92i0k10" "$d/n16d128c96t9338s15i0k10" "$d/n16d128c96t9338s65i0k10" "$d/n16d128c96t9338s31i0k10" "$d/n16d128c96t9338s69i0k10")
d='instances/vars12-vals256-defs10/urbcsp'
files+=("$d/n12d256c52t47382s100i0k10" "$d/n12d256c52t47382s93i0k10" "$d/n12d256c52t47382s34i0k10" "$d/n12d256c52t47382s7i0k10" "$d/n12d256c52t47382s75i0k10")
d='instances/vars10-vals512-defs5/urbcsp'
files+=("$d/n10d512c36t217579s55i1k5" "$d/n10d512c36t217579s99i1k5" "$d/n10d512c36t217579s67i1k5" "$d/n10d512c36t217579s9i1k5" "$d/n10d512c36t217579s45i1k5")
d='instances/vars10-vals1024-defs5/urbcsp'
files+=("$d/n10d1024c36t891289s63i1k5" "$d/n10d1024c36t891289s88i1k5" "$d/n10d1024c36t891289s78i1k5" "$d/n10d1024c36t891289s13i1k5" "$d/n10d1024c36t891289s55i1k5")

for i in $(seq 3); do
  for f in ${files[@]}; do
    ./bench.sh "$f" "${exed[*]/%/-d512}" ${exe[*]} ${exe[*]}
  done
done
