#!/bin/bash
set -eu -o pipefail
test $# == 1 || { echo "Usage: $0 <raw>" ; exit 1; }
raw=$1
play -c2 -b16 -e signed-integer -r44100 $raw
