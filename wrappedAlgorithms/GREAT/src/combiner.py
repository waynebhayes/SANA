#Ryan Solava
#
#June 15th, 2013
#
#Takes a GDV-similarity file and a GDV file and combines them for all alphas .1, .2, .3..., .9

import sys

def read_gdvs_file(file_name, centralities1, centralities2, max_score, alpha):
    file_handle = open(file_name, 'r')
    lines = file_handle.readlines()
    edges = lines.pop(0).split()
    
    outfiles = {} #used for files that will be plugged into the greedy function
    outfiles2 = {} #used for files that will be plugged into the Hungarian algorithm
    #alphas = ["00","01","02","03","04","05","06","07","08","09","10"]
    file = file_name.replace(".egdvs","")
    #for a in alphas:
    new_file = open(file + "_" + alpha + "_scores.txt", 'w') #for the greedy function
    new_file2 = open(file + "_" + alpha + "_scores_hung.txt", 'w') #for the Hungarian algorithm
    outfiles[alpha] = new_file
    outfiles2[alpha] = new_file2
    new_file.write(str(len(edge_list1)) + "\t" + str(len(edge_list2)) + "\n")
    new_file2.write(str(len(edge_list1)) + "\t" + str(len(edge_list2)) + "\n")

    p = 0
    for line in lines:
        p += 1
        terms = line.split()
        edge1 = terms.pop(0)
        n = 0
        if(p%100 == 0):
            print float(p)*100.0/len(lines)
        for val in terms:
            edge2 = edges[n]
            sim = float(val)
            cent1 = centralities1[edge1]
            cent2 = centralities2[edge2]
        
            #for a in alphas:
            score = sim*int(alpha)/10.0 + (cent1 + cent2)*(1-int(alpha)/10.0)/(max_score)
            outfiles[alpha].write(str(score) + "\t")
            outfiles2[alpha].write(str(1-score) + "\t")
            n += 1
        #for a in alphas:
        outfiles[alpha].write("\n")
        outfiles2[alpha].write("\n")
        
    #for a in alphas:
    outfiles[alpha].close()
    outfiles2[alpha].close()
    file_handle.close()
    return
    
def read_centrality_file(file_handle, cent_map, nodes):
    max_centrality = 0
    for line in file_handle.readlines():
        terms = line.split()
        node1 = terms[0]
        if( float(terms[2]) > max_centrality):
            max_centrality = float(terms[2])
        cent_map[node1] = float(terms[2])
    file_handle.close()
    return max_centrality
    
file_name1 = sys.argv[1]
file_name2 = sys.argv[2]
outfile_name = sys.argv[3]
alpha = sys.argv[4]

gdc_file1 = open(file_name1, 'r')
edge_list1 = [l.split()[0] for l in gdc_file1.readlines()]
gdc_file2 = open(file_name2, 'r')
edge_list2 = [l.split()[0] for l in gdc_file2.readlines()]
gdc_file1.close()
gdc_file2.close()

gdc_file1 = open(file_name1, 'r')
gdc_file2 = open(file_name2, 'r')

sim_map = {}
centralities1 = {}
centralities2 = {}

max_centrality1 = read_centrality_file(gdc_file1, centralities1, edge_list1)
print "Cent1 Loaded"
max_centrality2 = read_centrality_file(gdc_file2, centralities2, edge_list2)
print "Cent2 Loaded"
print "Max centrality score is: " + str(max_centrality1+max_centrality2)

read_gdvs_file(outfile_name, centralities1, centralities2, max_centrality1 + max_centrality2, alpha)

        
            
            
    
    




















