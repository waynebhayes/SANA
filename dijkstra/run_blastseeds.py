import argparse
#from research_for_graph import *
from graph_research import *
import seeding
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
    

    seed_length = seeding.get_seed_length(g1_seed_file)

    sims = get_sim(args.sim, graph1, graph2, args.pickle)

    for seed in seeding.generate_seed(g1_seed_file,g2_seed_file):
        
        n1, n2 = seed
        mat1 = seeding.adj_mat(n1,graph1)
        mat2 = seeding.adj_mat(n2,graph2)
        if mat1 != mat2:
            print(mat1)
            print(mat2)

        start = time.time()
        a, b, pairs = stop_align2(graph1, graph2, get_aligned_seed(zip(*seed),graph1, graph2), sims, delta)
        subgraph = induced_subgraph(graph1, graph2, list(pairs))
        cov = coverage(graph1, graph2, subgraph)[0]
        cov2 = coverage(graph1, graph2, subgraph)[1]

        print("Edges aligned in subgraph:",len(subgraph))
        print("Pairs aligned in subgraph:",len(pairs))
        newcov = round(cov, 2)
        newcov2 = round(cov2, 2)
        s3 = s3score(graph1, graph2, pairs, subgraph) 
        s3cov = round(s3, 2) 
        uuidstr = str(uuid.uuid4())
        uid = uuidstr[:13]
        
        fname = graph1.name + "--" + graph2.name + "--" + str(delta) + "--" + str(seed_length) + "--" + str(newcov) + "--"  + uid +  ".dijkstra"
        write_result(fname, pairs, graph1, graph2)
        end = time.time()
        hours, rem = divmod(end-start, 3600)
        minutes, seconds = divmod(rem, 60)
        runtime = "{:0>2}:{:0>2}:{:05.2f}".format(int(hours),int(minutes),seconds)
        #write_log(fname, runtime, seed, delta)
        logname = fname.replace("dijkstra", "log")
        with open(logname, 'w+') as logfile:
            logfile.write(fname + '\n')
            logfile.write(str(seed) + '\n')
            logfile.write(str(runtime) + '\n')
            logfile.write("delta: " + str(delta) + "\n")
            logfile.write("EC: " + str(cov) + "\n") 
            logfile.write("EC2: " + str(cov2) + "\n") 
            logfile.write("S3: " + str(s3cov) + "\n") 
            if mat1 != mat2:
                logfile.write("Seeds not matched" + "\n" )
                logfile.write(str(mat1) + "\n")
                logfile.write(str(mat2) + "\n")
