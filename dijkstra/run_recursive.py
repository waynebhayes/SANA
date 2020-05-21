import argparse
#from research_for_graph import *
#from graph_research import *
import recalignment
import seeding
import builder
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
    parser.add_argument("-ec1", "--ec1bound", required=False, default = "0.0", help ="lower bound for ec1")
    parser.add_argument("-ec2", "--ec2bound", required=False, default = "0.0", help ="lower bound for ec2")
    parser.add_argument("-s3", "--s3bound", required=False, default = "0.0", help ="lower bound for s3")
    parser.add_argument("-a", "--alpha", required=False, default = "1.0", help = "weight given to aligning based on local measurement")
    parser.add_argument("-b", "--beta", required=False, default = "", help = "weight given to aligning based on similarity matrix")
    parser.add_argument("-ed", "--edbound", required=False, default = "0.0", help = "edge density lower bound")
    parser.add_argument("-pk", "--pickle", required=False, default = "", help = "location of existing pickle file")
    parser.add_argument("-t", "--timestop", required=False, default = "-1.0", help = "Stop program after specified time, units in hours")
    parser.add_argument('-debug', "--debugval",action='store_true', help="adding debug will set to True, no entry is False")

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

    graph1 = builder.build_graph(args.graph1)
    graph1.name = os.path.basename(args.graph1)
    graph1.name = os.path.splitext(graph1.name)[0]
    graph2 = builder.build_graph(args.graph2)
    graph2.name = os.path.basename(args.graph2)
    graph2.name = os.path.splitext(graph2.name)[0] 

    g1_seed_file = args.g1seed
    g2_seed_file = args.g2seed
    
    ec_mode = (float(args.ec1bound), float(args.ec2bound), float(args.s3bound))
    ed = (float(args.edbound))
    alpha = float(args.alpha)
    seed_length = seeding.get_seed_length(g1_seed_file)


    timestop_arg = float(args.timestop)
    if timestop_arg < 0:
        timestop_arg = None
    else:
        #convert seconds to hours
        
        ntimestop = timestop_arg * 3600  
        timestop_arg = ntimestop

    print("timestop_arg: ", timestop_arg)

    sims = builder.get_sim(args.sim, graph1, graph2, args.pickle)

    seednum = 0
    
    seedpairs = 0 
    for seed in seeding.generate_seed(g1_seed_file,g2_seed_file):
        seedpairs += 1 
    print("Num of Seedpairs: " + str(seedpairs))

    for seed in seeding.generate_seed(g1_seed_file,g2_seed_file):
        seednum += 1 
        print("seednum: " + str(seednum) + " out of " + str(seedpairs))
        n1, n2 = seed
        mat1, e1 = seeding.adj_mat(n1,graph1)
        mat2, e2 = seeding.adj_mat(n2,graph2)
        if mat1 != mat2:
            print(mat1)
            print(mat2)

        recalignment.rec_align(graph1, graph2, seeding.get_aligned_seed(zip(*seed),graph1, graph2), sims, ec_mode, ed, e1, delta, alpha, seednum, timestop=timestop_arg, debug=args.debugval)    

