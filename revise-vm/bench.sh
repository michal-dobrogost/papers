#!/usr/bin/env bash

if [ $# -lt 2 ]; then
  >&2 echo "Usage: bench.sh path exe1 [exe2 ...]"
  exit 1
fi
path="$1"
shift 1
exe1="$1"

#for exe in "$@"; do
  #which $exe >/dev/null
  #if [ $? != 0 ]; then
    #>&2 echo "$exe: command not found"
    #exit 1
  #fi
#done

for file in $(find "$(dirname $path)" -type f | grep "$(basename $path).*\.json$" | sort); do
  file=$(realpath "$file")
  echo -n "{\"file\": \"$file\", "
  solutions=()
  solveds=()
  echo -n "\"results\": ["
  for exe in "$@"; do
    if [ "$exe" != "$exe1" ]; then
      echo -n ", "
    fi
    # Extract the exe basename (ie. drop the full path or the interpreter prefixes).
    exeName=$(basename "$exe")
    echo -n "{\"exe\": \"$exeName\", \"out\":"
    res=$($exe --csp "$file" 2>/dev/null | head -n 1 | tr -d '\n' | jq -c '.')
    echo -n $res
    echo -n "}"
    sol="$(echo "$res" | jq -c '.solution')"
    solutions+=( "$sol" )
    solved=$(cj-is-solved --csp $file --solution "$sol")
    solveds+=( "$solved" )
  done
  echo -n "]"

  same='true'
  for sol in ${solutions[@]}; do
    if [ "$sol" != "${solutions[0]}" ]; then
      same='false'
      break
    fi
  done
  echo -n ", \"same\": $same"

  echo -n ", \"solved\": "
  jq -cjnM '$ARGS.positional' --jsonargs -- "${solveds[@]}"
  echo "}"
done
