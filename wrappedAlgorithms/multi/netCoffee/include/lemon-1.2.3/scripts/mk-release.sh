#!/bin/bash
#
# This file is a part of LEMON, a generic C++ optimization library.
#
# Copyright (C) 2003-2009
# Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
# (Egervary Research Group on Combinatorial Optimization, EGRES).
#
# Permission to use, modify and distribute this software is granted
# provided that this copyright notice appears in all copies. For
# precise terms see the accompanying LICENSE file.
#
# This software is provided "AS IS" with no warranty of any kind,
# express or implied, and with no claim as to its suitability for any
# purpose.

set -e

if [ $# = 0 ]; then
    echo "Usage: $0 release-id"
    exit 1
else
    export LEMON_VERSION=$1
fi

echo '*****************************************************************'
echo ' Start making release tarballs for version '${LEMON_VERSION}
echo '*****************************************************************'

autoreconf -vif
./configure

make
make html
make distcheck
tar xf lemon-${LEMON_VERSION}.tar.gz
zip -r lemon-${LEMON_VERSION}.zip lemon-${LEMON_VERSION}
mv lemon-${LEMON_VERSION}/doc/html lemon-doc-${LEMON_VERSION}
tar czf lemon-doc-${LEMON_VERSION}.tar.gz lemon-doc-${LEMON_VERSION}
zip -r lemon-doc-${LEMON_VERSION}.zip lemon-doc-${LEMON_VERSION}
tar czf lemon-nodoc-${LEMON_VERSION}.tar.gz lemon-${LEMON_VERSION}
zip -r lemon-nodoc-${LEMON_VERSION}.zip lemon-${LEMON_VERSION}
hg tag -m 'LEMON '${LEMON_VERSION}' released ('$(hg par --template="{node|short}")' tagged as r'${LEMON_VERSION}')' r${LEMON_VERSION}

rm -rf lemon-${LEMON_VERSION} lemon-doc-${LEMON_VERSION}

echo '*****************************************************************'
echo '  Release '${LEMON_VERSION}' has been created' 
echo '*****************************************************************'
