"""
Gets CPU time of the alignment
usage: python time.py input.txt output.txt alignment_name.txt
"""

import sys

time_in = open(sys.argv[1],'r')
time_out = open(sys.argv[2],'a')

num=0
for a in time_in.readlines():
    a = a.strip()
    var = a.split()
    num += float(var[0]) + float(var[1])

time_out.write(sys.argv[3] + "-alignment " + str(num) + " CPU seconds" + "\n")    
