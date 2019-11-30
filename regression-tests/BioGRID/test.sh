#!/bin/sh
HOST=`hostname|sed 's/\..*//'`
./regression-tests/BioGRID/regression-test -c regression-tests/BioGRID/regression-test.baseline.$HOST
