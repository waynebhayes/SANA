"""
This code arbitrarily aligns nodes

In the edge alignment process, nodes who have no edges attached will not be aligned to each other in the
current version of the code. So this code finds all instances of unaligned nodes in the smaller network
(network 1), and aligns them to unaligned nodes in the larger network (network 2).

Usage: python isolated_nodes.py alignment.aln network1.gw network2.gw

NOTE: This only works when graphs are of equal size (in terms of nodes).
"""

import sys
from random import randrange

#Takes in the alignment file
aln = open(sys.argv[1],'r+')

#Takes in the network files
gw1 = open(sys.argv[2],'r')
gw2 = open(sys.argv[3],'r')

#Gets the number of nodes in the graph
for a in range(1,4):
    gw1.readline()
    gw2.readline()
num1 = gw1.readline()
num2 = gw2.readline()

#Stores the nodes that are not in the alignment, and stores the nodes used in the alignment
nodes1 = []
nodes2 = []
aln_nodes1 = []
aln_nodes2 = []
missing1 = []
missing2 = []

#Places all nodes in network 1 and 2 in a list
for a in range(0,int(num1)):
    b = gw1.readline()
    b = b.strip()
    b = b.replace("|{","")
    b = b.replace("}|","")
    nodes1.append(b)

for a in range(0,int(num2)):
    b = gw2.readline()
    b = b.strip()
    b = b.replace("|{","")
    b = b.replace("}|","")
    nodes2.append(b)
    
for a in aln.readlines():
    a = a.strip()
    var = a.split()
    aln_nodes1.append(var[0])
    aln_nodes2.append(var[1])


#Gets nodes not in the alignment file
for a in range(0,int(num1)):
    check1 = nodes1[a] in aln_nodes1
    check2 = nodes2[a] in aln_nodes2
    if check1 is False:
        missing1.append(nodes1[a])
    if check2 is False:
        missing2.append(nodes2[a])


#Places missing nodes into the alignment file
for a in range(0,len(missing1)):
    ran1 = randrange(len(missing1))
    ran2 = randrange(len(missing2))
    aln.write(missing1[ran1] + "\t" + missing2[ran2] +"\n")
    missing1.pop(missing1.index(missing1[ran1]))
    missing2.pop(missing2.index(missing2[ran2]))

aln.close()
gw1.close()
gw2.close()















