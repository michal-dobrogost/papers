#!/usr/bin/env bash

# These were used originally to set t ~= 0.5-0.9 and c from n.
#t=$(bc <<< "$d*$d*$t") # tightness as fraction
#c=$(bc <<< "$n*($n-1)*0.4") # num constraints from num vars

i='1'
./generate.sh 16 64   98 2048   10 "$i" # n16d64c98t2048s26i0k10
./generate.sh 16 128  96 9338   10 "$i" # n16d128c96t9338s87i0k10
./generate.sh 12 256  52 47382  10 "$i" # n12d256c52t47382s57i0k10
./generate.sh 10 512  36 217579 5  "$i" # n10d512c36t217579s42i1k5
./generate.sh 10 1024 36 891289 5  "$i" # n10d1024c36t891289s30i1k5

# note that some of the instances used in the "all" benchmark have i=0
# these are available at https://github.com/michal-dobrogost/csp-json-archive
