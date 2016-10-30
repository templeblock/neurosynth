#!/bin/bash
set -eu -o pipefail
test $# == 2 || { echo "Usage: $0 <wav> <raw>" ; exit 1; }
wav=$1
raw=$2
sox -c2 -b16 -e signed-integer -r44100 $wav $raw
