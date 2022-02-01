#!/bin/bash
OUTDIR=$1; shift
TYPES=false
myArray=( "$@" )


./random-multi-alignment.sh $TYPES $OUTDIR/.init "$@"

# helps call random-multi-alignment.sh which takes in a list of input networks