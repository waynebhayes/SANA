#!/usr/bin/env python
"""This program computes the list of signature similarities,
given two files with signatures

"""
import sys
import threading
import random
import math
class Signatures(threading.Thread):
    def __init__(self,filename):
        threading.Thread.__init__(self)
        self.sign_file=filename
        self.Sign=dict()
        #self.NodeNaming=dict() #pairs (node_number,node_name)
        #self.NodeNumber=0
    def readSignatures(self):
        inpt=open(self.sign_file,'r')
        self.NodeNumber=0
        for line in inpt:
            L=line.split()
            #self.NodeNumbering[L[0]]=self.NodeNumber
            #self.NodeNaming[self.NodeNumber]=L[0]
            self.Sign[L[0]]=[int(L[1])]
            for i in range(2,len(L)):
                self.Sign[L[0]].append(int(L[i]))
            #self.NodeNumber+=1
        inpt.close()
    def run(self):
        print("Reading signature file")
        self.readSignatures()
        
        
    
    
#program begins
if len(sys.argv)!=4:
    print("Usage: ./computeSignatures.py file1.ndump2 file2.ndump2 outputfile")
    exit()
    
    
S1=Signatures(sys.argv[1])
S2=Signatures(sys.argv[2])
S1.start()
S2.start()
print("Main thread waits for signatures to be read")
S1.join()
S2.join()
print("Signatures are loaded")
print("Computing signature similarities")
Similarities=dict() # I will store similarities in the dictionary the format of key is node1+"<-!->"+node2
num_of_orbits=73
weight_factor=[1, 2, 2, 2, 3, 4, 3, 3, 4, 3, 4, 4, 4, 4, 3, 4, 6, 5, 4, 5, 6, 6, 4, 4, 4, 5, 7, 4, 6, 6, 7, 4, 6, 6, 6, 5, 6, 7, 7, 5, 7, 6, 7, 6, 5, 5, 6, 8, 7, 6, 6, 8, 6, 9, 5, 6, 4, 6, 6, 7, 8, 6, 6, 8, 7, 6, 7, 7, 8, 5, 6, 6, 4]
sum_coef=0
weight=[]
for i in range(0,num_of_orbits):
    w=1-math.log10(weight_factor[i])/math.log10(num_of_orbits)
    weight.append(w)
    sum_coef+=weight[i]
print(len(S1.Sign))
print(len(S2.Sign))
progr=0.0
for key1, L1 in S1.Sign.items():
    for key2, L2 in S2.Sign.items():
        sim=0.0
        res=0.0
        max=-1.0
        for i in range(0,num_of_orbits):
            if L2[i]>L1[i]:
                max=L2[i]
            else:
                max=L1[i]
            res+=weight[i]*( math.fabs( math.log10( L1[i]+1) - math.log10( L2[i]+1) )  )/math.log10(max+2)
        sim=res/sum_coef
        new_key=str(key1)+"<-!->"+str(key2)
        Similarities[new_key]=sim
        progr+=1
    proc=progr/len(S1.Sign)
    print(str(proc)+" done")
out=open(sys.argv[3],'w')
for key, value in Similarities.items():
    out.write(key.replace('<-!->',' ')+" "+str(value)+'\n')
print("Done!")
