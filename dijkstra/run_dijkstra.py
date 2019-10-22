import argparse
#from research_for_graph import *
from graph_research import *
import lzma
import time
import os.path
import time
import uuid

def initParser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-g1" "--graph1", required=True, help ="first graph file")
    parser.add_argument("-g2" "--graph2", required=True, help ="second graph file")
    parser.add_argument("-s" "--sim", required=True, help ="sim file")
    parser.add_argument("-d" "--delta", required=True, help ="delta value")
    parser.add_argument("-r" "--runs", required=False, default = 1, help = "number of times to run")
    parser.add_argument("-pk" "--pickle", required=False, default = "", help = "location of existing pickle file")
    return parser


'''
def convertSim(sim_file,d1,d2):
    f = open("convertedSim.txt", 'w')
    for line in lzma.open(sim_file, mode = 'rt'):
        n1, n2, s = line.split()
        try:
            f.write(f"{d1[n1]} {d2[n2]} {s}\n")
        except:
            pass
    f.close()
'''

if __name__ == '__main__':
    parser = initParser()
    args = parser.parse_args()

    delta = float(args.d__delta)

    graph1 = build_graph(args.g1__graph1)
    graph1.name = os.path.basename(args.g1__graph1)
    graph1.name = os.path.splitext(graph1.name)[0]
    graph2 = build_graph(args.g2__graph2)
    graph2.name = os.path.basename(args.g2__graph2)
    graph2.name = os.path.splitext(graph2.name)[0]
    runs = int(args.r__runs)

    print(runs)
    
    sims = get_sim(args.s__sim, graph1, graph2, args.pk__pickle)
    fname = graph1.name + "--" + graph2.name + "--" + str(delta)
    
    print(fname)

    for i in range(runs):
        start = time.time()
        a, b, pairs = dijkstra(graph1, graph2, get_seed(args.s__sim, graph1, graph2, delta), sims, delta)

        subgraph = induced_subgraph(graph1, graph2, list(pairs))
        cov = coverage(graph1, graph2, subgraph)[0]

        newcov = round(cov,2)

        uuidstr = str(uuid.uuid4())
        uid = uuidstr[:8]

        fname = graph1.name + "--" + graph2.name + "--" + str(delta) + "-" + str(newcov) + "-" + str(i) + uid +  ".dijkstra"
        write_result(fname, pairs, graph1, graph2)
        
        end = time.time()
        hours, rem = divmod(end-start, 3600)
        minutes, seconds = divmod(rem, 60)
        print("{:0>2}:{:0>2}:{:05.2f}".format(int(hours),int(minutes),seconds))

