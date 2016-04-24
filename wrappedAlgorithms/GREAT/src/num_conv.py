"""
converts output from hung code so that it is usable in ealign2nscore
"""

import sys

#The matrix score file
mat=sys.argv[1]
matrix_file = open(mat,'r')

#Scales Matrix file between 0-1 such that the smaller the number, the better the alignment
hung_out= sys.argv[1].replace(".txt","_hung.txt")
hung_file = open(hung_out,'w')

#Scales Matrix file between 0-1 such that the higher the number, the better the alignment
greed_out = hung_out= sys.argv[1].replace(".txt","_greedy.txt")
greed_file = open(greed_out,'w')

#Gets dimension of the matrix file
dim = matrix_file.readline().split()
hung_file.write(dim[0] +"\t"+ dim[1])
greed_file.write(dim[0] +"\t"+ dim[1])

max_num = 0
for line in matrix_file.readlines():
    terms = line.split()
    for x in range(0,int(dim[1])):
        if( float(terms[x]) > max_num):
            max_num = float(terms[x])
matrix_file.close()

matrix_file = open(sys.argv[1],'r')
dim = matrix_file.readline().split()

#Places all values in the matrix file in a matrix
for a in matrix_file.readlines():
    hung_file.write("\n")
    greed_file.write("\n")
    var = a.split()
    for x in range(0,int(dim[1])):
        if max_num!=0:
            numg = float(var[x])/max_num
            num = 1 - numg
        else:
            numg = float(var[x])
            num = 1 - numg
        hung_file.write(str(num))
        greed_file.write(str(numg))
        if int(dim[1])-1!=x:
            hung_file.write("\t")
            greed_file.write("\t")









