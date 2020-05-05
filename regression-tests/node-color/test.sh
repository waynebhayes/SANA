#!/bin/sh

#generates a warning. this is intended.
# "Warning: some G2 nodes have a color non-existent in G1, so some G2 nodes won't be part of any valid alignment"
./sana -g1 yeast -g2 human -t 2 -s3 1 -fcolor1 regression-tests/node-color/yeast.col -fcolor2 regression-tests/node-color/human.col

#after running it:
grep PAP1 sana.align #check that it is aligned to LEP
grep RSC6 sana.align #check that it is aligned to either MYOC or PXN
grep CTSB sana.align #check that it does not appear


./sana -fg1 regression-tests/node-color/covid.el -fg2 regression-tests/node-color/covid.el -t 4 -s3 1 -fcolor1 regression-tests/node-color/covid.col -fcolor2 regression-tests/node-color/covid.col

#after running it:
grep -w 'E\|M\|N\|NSP1\|NSP10\|NSP11\|NSP12\|NSP13\|NSP14\|NSP15\|NSP2\|NSP4\|NSP5\|NSP5_C145A\|NSP6\|NSP7\|NSP8\|NSP9\|ORF10\|ORF3A\|ORF3B\|ORF6\|ORF7A\|ORF8\|ORF9B\|PROTEIN14\|S' sana.align
#these are all the virus nodes (color "virus"). Make sure none of them
#are aligned to a node starting with prefix "EN" (which are colored "human")



#terminates with a runtime error. this is intended.
# "there is a unique valid alignment, so running SANA is pointless"
#locking is implemented on top of the color system, so this tests the color system
./sana -fg1 regression-tests/node-color/covid.el -fg2 regression-tests/node-color/covid.el -t 0.3 -s3 1 -lock-same-names
