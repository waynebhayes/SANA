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
    parser.add_argument("-g1sline", "--g1seedline", required=False, help ="g1 seed file line")
    parser.add_argument("-g2sline", "--g2seedline", required=False, help ="g2 seed file")
    parser.add_argument("-ec1", "--ec1bound", required=False, default = "0.0", help ="lower bound for ec1")
    parser.add_argument("-ec2", "--ec2bound", required=False, default = "0.0", help ="lower bound for ec2")
    parser.add_argument("-s3", "--s3bound", required=False, default = "0.0", help ="lower bound for s3")
    parser.add_argument("-a", "--alpha", required=False, default = "1.0", help = "weight given to aligning based on local measurement")
    parser.add_argument("-b", "--beta", required=False, default = "", help = "weight given to aligning based on similarity matrix")
    parser.add_argument("-sb", "--simbound", required=False, default = "0.0", help = "lower bound for similarity measure")
    parser.add_argument("-ed", "--edbound", required=False, default = "0.0", help = "edge density lower bound")
    parser.add_argument("-pk", "--pickle", required=False, default = "", help = "location of existing pickle file")
    parser.add_argument("-t", "--timestop", required=False, default = "-1.0", help = "Stop program after specified time, units in hours")
    parser.add_argument("-sn", "--seednum", required=False, default = "", help = "seed num")
    parser.add_argument('-debug', "--debugval",action='store_true', help="adding debug will set to True, no entry is False")

    return parser

if __name__ == '__main__':
    parser = initParser()
    args = parser.parse_args()

    delta = float(args.delta)

    graph1 = builder.build_graph(args.graph1)
    graph1.name = os.path.basename(args.graph1)
    print(graph1.name)
    graph1.name = os.path.splitext(graph1.name)[0]
    graph2 = builder.build_graph(args.graph2)
    graph2.name = os.path.basename(args.graph2)
    graph2.name = os.path.splitext(graph2.name)[0] 

    g1_seedstr = args.g1seed
    g2_seedstr = args.g2seed

    ec_mode = (float(args.ec1bound), float(args.ec2bound), float(args.s3bound))
    ed = (float(args.edbound))
    alpha = float(args.alpha)
    seednum = int(args.seednum)
    simbound = float(args.simbound)
    sims = builder.get_sim(args.sim, graph1, graph2, args.pickle)
    g1seed = seeding.get_seed_line(args.g1seed, args.g1seedline)
    g2seed = seeding.get_seed_line(args.g2seed, args.g2seedline)
    assert g1seed[0] == g2seed[0], "Kval not matching"
    g1seed = g1seed[1:] 
    g2seed = g2seed[1:] 
    print(g1seed)
    print(g2seed)

    seed = seeding.get_aligned_seed(zip(g1seed,g2seed), graph1, graph2) 
    print(seed)
    mat1, e1 = seeding.adj_mat(g1seed,graph1)
    timestop_arg = float(args.timestop)
    if timestop_arg < 0:
        timestop_arg = None
    else:
        ntimestop = timestop_arg * 3600  
        timestop_arg = ntimestop


    recalignment.rec_align(graph1, graph2, seed, sims, ec_mode, ed, e1, simbound, delta, alpha, seednum, timestop=timestop_arg, debug=args.debugval)    