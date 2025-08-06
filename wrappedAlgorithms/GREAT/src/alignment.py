"""
Usage: python Alignment.py row_col.aln labels.txt alignment.aln

labels.txt must be in the following format:
Let A represent the nodes in network A, and B the nodes in network B
A A A A A A
B B B B B B

"""

import sys

#This file shows the alignment by their column and row number
in_aln = open(sys.argv[1],'r')
#This file contains teh labels for the alignment
in_label = open(sys.argv[2],'r')
#This file outputs the alignment by name
out_aln = open(sys.argv[3],'w')

"""
Gets all the nodes in network B (resp. A)
In the order listed in the labels file, the program will assign it a number from 0
to network B's (resp. A) size minus 1
"""
tempB = in_label.readline()
tempA = in_label.readline()
network_B = tempB.split()
network_A = tempA.split()

#matches the labels with the row and column number
for a in in_aln.readlines():
    align = a.split()
    node_A = network_A[int(align[0])-1]
    node_B = network_B[int(align[1])-1]
    out_aln.write(node_A + "\t" + node_B + "\n")

out_aln.close()
    
    




