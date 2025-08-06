"""
./ecounts does not return an edge if it is isolated
This program finds all isolated edges and adds them to the file output for
ecounts that is used for edge alignment

the n2dumps file lists all edges in the in the graph. In this file, at each row, it
lists a node and the number of edges it touches. If it only touches one other node, check
the remainder of that row. If that row is all 0's, then that node is excluded from the final
output of ./ecounts.

This program finds that excluded edges, then adds the edge to the output of ecounts with trailing 0's
for all the orbits

usage: python edge_counts_check.py dumps_file.ndump2 ecounts_file.ecounts network.gw
"""

import sys

#Takes in the dumps file
dump = open(sys.argv[1],'r')

#Takes in the output file (.ecounts) from ./ecount
counts = open(sys.argv[2],'a')

#Takes in the .gw file
gw = open(sys.argv[3],'r')

#Stores the nodes who participate in an isolated edge, and their representation in the .gw file
excluded = []
position = []

#Finds nodes who participate in an isolated edge
for a in dump.readlines():
    var = a.split()
    if var[1] == "1":
        check = 0
        for b in range(2,len(var)):
            if var[b]!="0":
                check+=1
        if check==0:
            excluded.append(var[0])

#Gets the number of nodes in the graph, so it can jump down to the edges when searching
for a in range(1,4):
    gw.readline()
nodes = gw.readline()

#skips down to the number of edges
for a in range(1,int(nodes)+1):
    gw.readline()

#Stores number of lines to skip to get the number of edges for later
edges = int(nodes) + 5

gw.close()

#finds the edge the excluded node participates in
for b in range(0,len(excluded)):
    gw = open(sys.argv[3],'r')
    check = -4
    for a in gw.readlines():
        check+=1
        a = a.strip()
        a = a.replace("|{","")
        a = a.replace("}|","")
        if a == excluded[b]:
            position.append(check)
            break
    gw.close()

print "The following nodes were excluded (if any)"
for a in range (0,len(excluded)):
    print excluded[a]
    

while len(excluded)!=0:
    gw = open(sys.argv[3],'r')
    #skips down to edges    
    for b in range(1,int(nodes)+6):
        gw.readline()
    #searches for edge    
    for b in gw.readlines():
        var = b.split()
        #if it finds the edge, remove the nodes from excluded/position list and add edge to ecounts file
        if var[0]==str(position[0]) or var[1]==str(position[0]):
            pos1 = position.index(int(var[0]))
            pos2 = position.index(int(var[1]))
            counts.write(excluded[pos1] + "-" + excluded[pos2] + " ")
            for d in range(1,70):
                if d!=69:
                    counts.write("0" + " ")
                else:
                    counts.write("0")  
            counts.write("\n")
            position.pop(int(pos1))
            excluded.pop(int(pos1))
            #finds the position again, since indices change with the above removal
            pos2 = position.index(int(var[1]))            
            excluded.pop(int(pos2))
            position.pop(int(pos2))
            break


            
        
        
    
        
    
    






























