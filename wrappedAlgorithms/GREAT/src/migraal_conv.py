"""
Converts the greedy scores produced by from ealign_sum/ealign_num into a format accepted
by MI-GRAAL
"""

import sys

#Greedy node scores - node_score file
greed = open(sys.argv[1],'r')

#Node names - label file
nodes = open(sys.argv[2],'r')

#Score format accepted by MI-GRAAL
migraal = open(sys.argv[3],'w')

#Gets the dimensions of the matrix, i.e the number of nodes in both networks
dim = greed.readline().split()

#Gets the node names of the second network in order
network2 = nodes.readline().split()

#Gets the node names of the first network in order
network1 = nodes.readline().split()

#Reads the greed file line by line. Associates the score with the node names of both networks
inc = 0
for aln in greed.readlines():
    align = aln.split()
    for b in range(0,int(dim[1])):
        migraal.write(network1[inc] + "\t" + network2[b] + "\t" + str(align[b]) + "\n")
    inc+=1

greed.close()
nodes.close()
migraal.close()
