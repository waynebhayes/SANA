#!/bin/sh
grep '^[ 	]*{[^}]*}' supportedArguments.cpp | sed -e 's/{//g' -e 's/}[ 	]*,[ 	]*$//' > argumentTable.csv
