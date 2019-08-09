#!/bin/sh
PATH="`pwd`/scripts:$PATH"
export PATH
DIR=/tmp/syeast
/bin/rm -rf $DIR
./multi-pairwise.sh ./sana.multi '-s3 0 -ses 1' 10 1 '-parallel 4' $DIR networks/syeast[12]?/*.el
