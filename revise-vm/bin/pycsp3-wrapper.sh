#!/usr/bin/env bash
exe='/Users/mkd/src/csp-json-pycsp3/cj-solve-pycsp3.py'
"$exe" $@ | egrep -v 'Creation|Warning'
