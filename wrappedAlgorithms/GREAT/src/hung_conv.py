"""
converts output from hung code so that it is usable in ealign2nscore
Usage: python hung_conv.py hung_aln.aln matrix_score.txt alignment.aln
"""

import sys

#Alignment file produced by hung code
hung_file = open(sys.argv[1],'r')

#Matrix file of scores
matrix_file = open(sys.argv[2],'r')

#Converted file
aln_file = open(sys.argv[3],'w')

#Gets dimension of the matrix file
dim = matrix_file.readline().split()
matrix={}

size = int(dim[1])
inc = 0
#Places all values in the matrix file in a matrix
for a in matrix_file.readlines():
    matrix[inc]={}
    var = a.split()
    for x in range(0,size):
        matrix[inc][x]=var[x]
    inc+=1

#Reads in the alignment from the hung file. 
#Outputs conversion to alignment file
#Adds one to all matrix rows/cols to function properly in ealign2nscore
line = int(hung_file.readline())

num=1
for b in range(0,line):
    c = hung_file.readline()
    term = int(c) + 1
    aln_file.write(str(num) + "\t" + str(term) + "\t" + matrix[num-1][term-1] + "\n" )
    num+=1
aln_file.close()











