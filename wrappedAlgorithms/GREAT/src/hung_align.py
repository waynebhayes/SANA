"""
converts output from hung code so that it is usable in ealign2nscore
Usage: python hung_conv.py hung_aln.aln matrix_score.txt alignment.aln
"""

import sys

#Alignment file produced by hung code
hung_file = open(sys.argv[1],'r')

#label file
in_label = open(sys.argv[2],'r')

#Converted file
aln_file = open(sys.argv[3],'w')


"""
Gets all the nodes in network B (resp. A)
In the order listed in the labels file, the program will assign it a number from 0
to network B's (resp. A) size minus 1
"""
tempB = in_label.readline()
tempA = in_label.readline()
network_B = tempB.split()
network_A = tempA.split()

#Reads in the alignment from the hung file. 
#Outputs conversion to alignment file


#matches the labels with the row and column number
num = int(hung_file.readline())
inc = 0
for x in range(0,num):
    align = int(hung_file.readline())
    node_A = network_A[inc]
    node_B = network_B[align]
    aln_file.write(node_A + "\t" + node_B + "\n")
    inc+=1

aln_file.close()















