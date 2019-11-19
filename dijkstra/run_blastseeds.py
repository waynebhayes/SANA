import argparse
#from research_for_graph import *
from graph_research import *
import lzma
import time
import os.path
import time
import uuid
from collections import defaultdict

def initParser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-g1", "--graph1", required=True, help ="first graph file")
    parser.add_argument("-g2", "--graph2", required=True, help ="second graph file")
    parser.add_argument("-s", "--sim", required=False, help ="sim file")
    parser.add_argument("-d", "--delta", required=True, help ="delta value")
    parser.add_argument("-r", "--runs", required=False, default = 1, help = "number of times to run")
    parser.add_argument("-g1s", "--g1seed", required=False, help ="g1 seed file")
    parser.add_argument("-g2s", "--g2seed", required=False, help ="g2 seed file")
    parser.add_argument("-pk", "--pickle", required=False, default = "", help = "location of existing pickle file")
    return parser


def get_g1_seed(g1_seed_file) -> dict:
    g1_seed = defaultdict(list)
    for line in open(g1_seed_file):
        line = line.split()
        g1_seed[line[0]].append(line[1:])
    return g1_seed

def generate_seed(g1_seed_file, g2_seed_file):
    g1_seed = get_g1_seed(g1_seed_file)
    #print (g1_seed)
    for line in open(g2_seed_file):
        line = line.split()
        if line[0] in g1_seed:
            for nodes in g1_seed[line[0]]:
                #print (nodes, line[1:])
                yield nodes,line[1:]

def get_seed_length(network) -> int:
    #RNorvegicus_5_30_300000_MAX.txt 
    info = network.split("/")
    data = info[-1].split("_")
    return data[1]

def write_log(fname, runtime, seed, delta):
    logname = fname.replace("dijkstra", "log")
    with open(logname, 'w+')as f:
        f.write(fname + '\n')
        f.write(str(seed) + '\n')
        f.write(str(runtime) + '\n')
        f.write("delta: " + str(delta) + "\n")

if __name__ == '__main__':
    parser = initParser()
    args = parser.parse_args()

    delta = float(args.delta)

    graph1 = build_graph(args.graph1)
    graph1.name = os.path.basename(args.graph1)
    graph1.name = os.path.splitext(graph1.name)[0]
    graph2 = build_graph(args.graph2)
    graph2.name = os.path.basename(args.graph2)
    graph2.name = os.path.splitext(graph2.name)[0]
    

    g1_seed_file = args.g1seed
    g2_seed_file = args.g2seed
    
    #print (len(graph1), len(graph2))

    seed_length = get_seed_length(g1_seed_file)
    #print("seed length")
    #print(seed_length)

    sims = get_degree_diff_matrix(graph1, graph2)
    #sims = degree_diff_matrix(graph1, graph2)
    
    for seed in generate_seed(g1_seed_file,g2_seed_file):
        start = time.time()
        a, b, pairs = dijkstra(graph1, graph2, get_aligned_seed(zip(*seed), graph1, graph2), sims, delta)
        subgraph = induced_subgraph(graph1, graph2, list(pairs))
        cov = coverage(graph1, graph2, subgraph)[0]
        #print(subgraph)
        #print(cov)
        print("Nodes aligned: "+str(len(subgraph)))
        newcov = round(cov, 2)
        
        uuidstr = str(uuid.uuid4())
        uid = uuidstr[:13]
        
        fname = graph1.name + "--" + graph2.name + "--" + str(delta) + "--" + str(seed_length) + "--" + str(newcov) + "--"  + uid +  ".dijkstra"
        write_result(fname, pairs, graph1, graph2)
        end = time.time()
        hours, rem = divmod(end-start, 3600)
        minutes, seconds = divmod(rem, 60)
        runtime = "{:0>2}:{:0>2}:{:05.2f}".format(int(hours),int(minutes),seconds)
        write_log(fname, runtime, seed, delta)
       

#    for i in range(runs):
#        start = time.time()
#        a, b, pairs = dijkstra(graph1, graph2, get_seed(args.s__sim, graph1, graph2, delta), sims, delta)
#
#        subgraph = induced_subgraph(graph1, graph2, list(pairs))
#        cov = coverage(graph1, graph2, subgraph)[0]
#
#        newcov = round(cov,2)
#
#        uuidstr = str(uuid.uuid4())
#        uid = uuidstr[:8]
#
#        fname = graph1.name + "--" + graph2.name + "--" + str(delta) + "-" + str(newcov) + "-" + str(i) + uid +  ".dijkstra"
#        write_result(fname, pairs, graph1, graph2)
#        
#        end = time.time()
#        hours, rem = divmod(end-start, 3600)
#        minutes, seconds = divmod(rem, 60)
#        print("{:0>2}:{:0>2}:{:05.2f}".format(int(hours),int(minutes),seconds))
#
