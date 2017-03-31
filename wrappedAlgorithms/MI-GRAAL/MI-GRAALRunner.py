#!/usr/bin/env python
import sys
import os
import random
import threading

class parallel_runner(threading.Thread): #this is used to run 2 ncounts in parallel
    def __init__(self,command):
        threading.Thread.__init__(self)
        self.command=command
    def run(self):
        print("Running command in separate thread")
        os.system(self.command)


if not (len(sys.argv)==6 or len(sys.argv)==8 or len(sys.argv)==10):
    print("Usage: ./MI-GRAALRunner.py net1 net2  result -p p [-q sequence_sim_file] [-s seed]")
    print("The number of nodes in the net1 should be <= number of nodes in net2!")
    print("Due to the inherent randomness in the algorithm it may produce different results")
    print("among different runs. Use -s option to specify the seed for random number generator")
    print("""The parameter p specifies which cost matrices should be used:
1 signature similarites
2 degrees
4 clustering coefficients
8 eccentricities
16 sequence scores, or any other matrix. This matrix should be specified as a pairs - score file (each line: [node 1] [node 2] [score]), using parameter -q
32 betweeness centralitites.
To use several cost matrices simultaneously, just add corresponding parameter values. For example, if you want to use signatures (1) and degrees (2) run it with -p 3")
If you want to use signatures (1), sequences (16) and degrees (2) I run it with -p 19

Simpliest example: "./MI-GRAALRunner.py testGraph1.gw testGraph2.gw res -p 3"
It will align testGraph1.gw and testGraph2.gw using signatures (1) and relative degrees (2)
""")
    exit(1)

rndnum=random.randint(1,100000000)
dirpath="tmp"+str(rndnum)
os.system("mkdir "+dirpath)
L1=sys.argv[1].split("/")
L2=sys.argv[2].split("/")
file1=L1[len(L1)-1]
file2=L2[len(L2)-1]

runner1=parallel_runner("./ncount "+sys.argv[1]+" "+dirpath+"/"+file1+".res")
runner2=parallel_runner("./ncount "+sys.argv[2]+" "+dirpath+"/"+file2+".res")

runner1.start()
runner2.start()
runner1.join()
runner2.join()


if (len(sys.argv)==6):
    os.system("./MI-GRAAL "+sys.argv[1]+" "+sys.argv[2]+" "+dirpath+"/"+file1+".res.ndump2 "+dirpath+"/"+file2+".res.ndump2 "+sys.argv[3]+" -p " +sys.argv[5]) # -p
elif (len(sys.argv)==8):
    os.system("./MI-GRAAL "+sys.argv[1]+" "+sys.argv[2]+" "+dirpath+"/"+file1+".res.ndump2 "+dirpath+"/"+file2+".res.ndump2 "+sys.argv[3]+" -p " +sys.argv[5]+ " -q "+ sys.argv[7]) # -q
elif (len(sys.argv)==10):
    os.system("./MI-GRAAL "+sys.argv[1]+" "+sys.argv[2]+" "+dirpath+"/"+file1+".res.ndump2 "+dirpath+"/"+file2+".res.ndump2 "+sys.argv[3]+" -p " +sys.argv[5]+ " -q "+ sys.argv[7]+" -s " +sys.argv[9]) # -s


os.system("rm -f -r "+dirpath)



