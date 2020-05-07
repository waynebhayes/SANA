#!/bin/sh

#generates a warning. this is intended.
# "Warning: some G2 nodes have a color non-existent in G1, so some G2 nodes won't be part of any valid alignment"
"$SANA_EXE" -g1 yeast -g2 human -t 2 -s3 1 -fcolor1 regression-tests/node-color/yeast.col -fcolor2 regression-tests/node-color/human.col

#after running it:
grep PAP1 sana.align #check that it is aligned to LEP
grep RSC6 sana.align #check that it is aligned to either MYOC or PXN
grep CTSB sana.align #check that it does not appear


"$SANA_EXE" -fg1 regression-tests/node-color/covid.el -fg2 regression-tests/node-color/covid.el -t 4 -s3 1 -fcolor1 regression-tests/node-color/covid.col -fcolor2 regression-tests/node-color/covid.col

#after running it:
grep -w 'E\|M\|N\|NSP1\|NSP10\|NSP11\|NSP12\|NSP13\|NSP14\|NSP15\|NSP2\|NSP4\|NSP5\|NSP5_C145A\|NSP6\|NSP7\|NSP8\|NSP9\|ORF10\|ORF3A\|ORF3B\|ORF6\|ORF7A\|ORF8\|ORF9B\|PROTEIN14\|S' sana.align
#these are all the virus nodes (color "virus"). Make sure none of them
#are aligned to a node starting with prefix "EN" (which are colored "human")



#terminates with a runtime error. this is intended.
# "there is a unique valid alignment, so running SANA is pointless"
#locking is implemented on top of the color system, so this tests the color system
"$SANA_EXE" -fg1 regression-tests/node-color/covid.el -fg2 regression-tests/node-color/covid.el -t 0.3 -s3 1 -lock-same-names


#in this test:
#G1 has 1 node colored c0, 1 colored c1, 2 colored c2, 3 colored c3, and 3 with default color
#G2 has 1 node colored c0, 2 colored c1, 2 colored c2, 3 colored c3, and 7 with default color
#without debug prints commented out, SANA shows that it is counting neighbors correctly:
# color __default has 3 swap nbrs and 12 change nbrs (15 total)
# color c0 has 0 swap nbrs and 0 change nbrs (0 total)
# color c1 has 0 swap nbrs and 1 change nbrs (1 total)
# color c2 has 1 swap nbrs and 0 change nbrs (1 total)
# color c3 has 3 swap nbrs and 0 change nbrs (3 total)
# an alignment has 20 nbrs in total
# color __default (id 0) has prob 0.75 (accumulated prob is now up to 0.75)
# color c0 (id 4) is inactive
# color c2 (id 1) has prob 0.05 (accumulated prob is now up to 0.8)
# color c1 (id 2) has prob 0.05 (accumulated prob is now up to 0.85)
# color c3 (id 3) has prob 0.15 (accumulated prob is now up to 1)
"$SANA_EXE" -fg1 regression-tests/node-color/colorTest1.el -fcolor1 regression-tests/node-color/colorTest1.col -fg2 regression-tests/node-color/colorTest2.el -fcolor2 regression-tests/node-color/colorTest2.col -s3 1 -t 2

more sana.align
#check the following matches:
# v0 -> w8
# v1 -> {w2, w3}
# {v2, v3} -> {w0, w1}
# {v4, v5, v6} -> {w7, w9, w10}


#locking test (implemented on top of color system)
#all the lock colors should be marked as inactive
# color __default (id 0) has prob 1 (accumulated prob is now up to 1)
# color lock_3 (id 1) is inactive
# color lock_2 (id 2) is inactive
# color lock_1 (id 3) is inactive
# color lock_0 (id 4) is inactive
"$SANA_EXE" -fg1 regression-tests/node-color/colorTest1.el -fg2 regression-tests/node-color/colorTest2.el -lock regression-tests/node-color/lockTest.lock -s3 1 -t 2

more sana.align 
#check the following matches:
# v1 w1
# v2 w3 
# v3 w2
# v7 w7
