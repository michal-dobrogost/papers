#!/usr/bin/env bash
exe="$(dirname $0)/../src/cj-pycsp3/cj-solve-pycsp3.py"
"$exe" $@ | egrep -v 'Creation|Warning'
