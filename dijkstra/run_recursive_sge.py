import argparse
import subprocess
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
    parser.add_argument("-sb", "--simbound", required=False, default = "0.0", help = "lower bound for similarity measure")
    parser.add_argument("-ed", "--edbound", required=False, default = "0.0", help = "edge density lower bound")
    parser.add_argument("-pk", "--pickle", required=False, default = "", help = "location of existing pickle file")
    parser.add_argument("-t", "--timestop", required=False, default = "-1.0", help = "Stop program after specified time, units in hours")
    parser.add_argument("-at", "--alignstop", required=False, default = -1, help = "Stop program after speciefied number of alignments generated")
    parser.add_argument("-od", "--outputdir", required=False, default = "", help = "outputdir")
    parser.add_argument('-debug', "--debugval",action='store_true', help="adding debug will set to True, no entry is False")
    return parser


if __name__ == '__main__':
    parser = initParser()
    args = parser.parse_args()

    delta = float(args.delta)

    g1_seed_file = args.g1seed
    g2_seed_file = args.g2seed
    ec_mode = (float(args.s3bound), float(args.ec2bound), float(args.ec1bound))
    ed = (float(args.edbound))
    alpha = float(args.alpha)
    seed_length = seeding.get_seed_length(g1_seed_file)
    simbound = float(args.simbound)

    timestop_arg = float(args.timestop)
    if timestop_arg < 0:
        timestop_arg = None
    else:
        #convert seconds to hours
        ntimestop = timestop_arg * 3600  
        timestop_arg = ntimestop

    

    seednum = 0
    for g1linenum, g2linenum in seeding.generate_seedinfo(g1_seed_file, g2_seed_file):
        seednum += 1 
        if args.outputdir != "":
            pod = "-od " + args.outputdir + " "
        else:
            pod = ""
        pgraph1 =  "-g1 "+ args.graph1+ " "
        pgraph2 =  "-g2 "+ args.graph2+ " " 
        pg1s = "-g1s "+ g1_seed_file+ " " 
        pg2s = "-g2s "+ g2_seed_file+ " " 
        pg1linenum = "-g1sline "+ str(g1linenum)+ " " 
        pg2linenum = "-g2sline "+ str(g2linenum)+ " " 
        psim = "-s " + args.sim+ " " if args.sim else ""
        pdelta = " -d 0.0 " 
        pec1 = "-ec1 "+ str(ec_mode[1])
        pec2 = "-ec2 "+ str(ec_mode[2])
        ped = "-ed "+ str(ed)+ " "
        psb = "-sb "+ str(simbound)+ " "
        psn = "-sn " + str(seednum) + " " 
        pt = "-t " + args.timestop + " "
        pat = "-at " + args.alignstop + " "
        ppk = " -pk "+ args.pickle+ " " if args.pickle else ""
        path = subprocess.run(['pwd'], stdout=subprocess.PIPE).stdout.decode('utf-8')
        prog = "python3.7 " + path.strip() + "/run_recursive_seed.py "
        print(prog, pgraph1, pgraph2, pg1s, pg2s, pg1linenum, pg2linenum, psim, pdelta, pec1, pec2, ped, psb, pt, pat, pod, psn, ppk) 
