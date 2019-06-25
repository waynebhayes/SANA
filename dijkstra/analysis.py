# analysis.py

import matplotlib.pyplot as plt
import os

#result = [[] for i in range(9)]
d = {}
for i in range(9):
    d[i**2*0.0025] = i


for f in os.listdir(r"D:\Documents\sophomore\CS 199\Dijkstra's algorithm\randomness"):
    with plt.style.context('Solarize_Light2'):
        if '-' in f:
            result = [[] for i in range(9)]
            for line in open(f):
                line = line.split(',')
                delta = eval(line[2])
                cov = eval(line[3])
                result[d[delta]].append(cov)
            plt.plot([result[0][0]]+[sum(result[i])/len(result[i]) for i in range(9)], label = f)
            plt.plot([result[0][0]]+[max(result[i]) for i in range(9)], linestyle='dashed', label = f+"max")
            
            plt.boxplot(result)
##            flag = False
##            for i in range(1,9):
##                b = sum(result[i])/len(result[i]) - sum(result[i-1])/len(result[i-1])
##                if b < 0:
##                    flag = True
##            if flag:
##                print(f)
##                plt.boxplot(result)
##                #plt.plot([sum(result[i])/len(result[i]) for i in range(9)], label = f)
            plt.title(f)
            plt.ylabel("edge_coverage")
            plt.xlabel("delta= (i-1)^2 * 0.0025")
            plt.legend()
            plt.savefig(f[:-3]+"png")
            plt.figure()
 
