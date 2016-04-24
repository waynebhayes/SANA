"""
Converts GDV-similarity scores generated from Graphcrunch to be used in
the greedy and Hungarian alignment strategies. 
Assumes that you are aligning nodes, not edges
Changes the scores from CSV, to matrix form.

Graphcrunch 2 outputs scores in a way such that the scores produced for a single nodes are outputted in sequential order
e.g:   "1","1","0.1"
	"1","2","0.456"
	"1","3","1"

usage: python network1.gw network2.gw scores matrix_name node_names
Network1 is the smaller network (rows in the matrix), network 2 is the larger network (columns in the matrix)
Node names puts the node names in a format accepted by the code that generates the final alignment.
"""

import sys

#Gets .gw files
gw1 = sys.argv[1]+".gw"
gw2 = sys.argv[2]+".gw"
GW1_file = open(gw1,'r')
GW2_file = open(gw2,'r')

#GDV-Similarity Scores
gdv = sys.argv[3]
print gdv
GDV_file = open(gdv,'r')

#Output of converted scores (matrix form)
conv = sys.argv[4]
conv_file = open(conv,'w')

#Outputs the name of each node
names = sys.argv[5]
name_file = open(names,'w')

#gets dimensions of the matrix
#Finds the number of nodes in both gw files. Assumed that the number of nodes is on the 4th line in the file
GW1_file.readline()
GW2_file.readline()
GW1_file.readline()
GW2_file.readline()
GW1_file.readline()
GW2_file.readline()
num1 = GW1_file.readline()
num2 = GW2_file.readline()
num1 = num1.strip()
num2 = num2.strip()

#Gets the name of the nodes in network 1
for a in range(0,int(num1)):
    line = GW1_file.readline()
    line = line.strip()
    line = line.replace("|{","")
    line = line.replace("}|","")
    name_file.write(line + "\t")
name_file.write("\n")

#Gets the name of the nodes in network 2
for a in range(0,int(num2)):
    line = GW2_file.readline()
    line = line.strip()
    line = line.replace("|{","")
    line = line.replace("}|","")
    name_file.write(line + "\t")
name_file.close()

conv_file.write(num1 + "\t" + num2 + "\n")
GDV_file.close()

#Places scores in matrix format
GDV_file = open(sys.argv[3],'r')
row=0
for b in GDV_file.readlines():
    b = b.strip()
    b = b.replace('"',"")
    var = b.split(',')
    conv_file.write(var[2] + "\t")
    row+=1
    if row==int(num2):
        conv_file.write("\n")
        row=0


    
        
        
    
