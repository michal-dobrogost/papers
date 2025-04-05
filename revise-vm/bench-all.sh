#!/usr/bin/env bash

exed=(./cj-solve-csp01 ./cj-solve-csp01-1bit)
exedneon=(./cj-solve-csp01)
exe=(./cj-solve-csp01 pycsp3-wrapper.sh../csp-json-cplex/build/cj-solve-cplex ../csp-json-or-tools/build/cj-solve-or-tools-cp)
#../csp-json-or-tools/build/cj-solve-or-tools-cpsat
#../csp-gecode/build/cj-solve-gecode

for i in $(seq 5); do
  ./bench.sh ../csp-json-archive/data/urbcsp/n16d64c98t2048s26i0k10 ${exed[*]/%/-d64} ${exe[*]}
  ./bench.sh ../csp-json-archive/data/urbcsp/n16d128c96t9338s87i0k10 ${exed[*]/%/-d128} ${exe[*]} './cj-solve-csp01-neon-d128'
  ./bench.sh ../csp-json-archive/data/urbcsp/n12d256c52t47382s57i0k10 ${exed[*]/%/-d256} ${exe[*]} './cj-solve-csp01-neon-d128'
  ./bench.sh ../csp-json-archive/data/urbcsp/n10d512c36t217579s42i1k5 ${exed[*]/%/-d512} ${exe[*]} './cj-solve-csp01-neon-d128'
  ./bench.sh ../csp-json-archive/data/urbcsp/n10d1024c36t891289s30i1k5 ${exed[*]/%/-d1024} ${exe[*]} './cj-solve-csp01-neon-d128'
done
