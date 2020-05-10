


./sana -fg1 regression-tests/dummies/testG1.el -fg2 regression-tests/dummies/testG2.el -fcolor1 regression-tests/dummies/testG1.col -fcolor2 regression-tests/dummies/testG2.col -ec 1 -t 0.1 -tinitial 1 -tdecay 1
# adding 2 dummies colored red to G2
# adding 1 dummies colored blue to G2
# adding 1 dummies colored __default to G2
# Warning: some G2 nodes have a color non-existent in G1, so some G2 nodes won't be part of any valid alignment

#sana.align:
# 0       dummy_0_red
# 1       15
# 2       14
# 3       dummy_1_red
# 4       dummy_0_blue    <-- only choice for 4 
# 5       dummy_0         <-- only choice for 5
