#!/bin/bash
HOST=`hostname|sed 's/\..*//'`
[ -f regression-tests/BioGRID/regression-test.baseline.$HOST ] || HOST=Jenkins
./regression-tests/BioGRID/regression-test -c regression-tests/BioGRID/regression-test.baseline.$HOST
