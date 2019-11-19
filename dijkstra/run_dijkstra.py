import argparse
#from research_for_graph import *
from graph_research import *
import lzma
import time
import os.path
<<<<<<< HEAD
import time
import uuid
||||||| merged common ancestors
=======
import time
import uuid
from collections import defaultdict
>>>>>>> 9d46a06f71c587b60383c55831059bd6b7c8ed73

def initParser():
<<<<<<< HEAD
    parser = argparse.ArgumentParser()
    parser.add_argument("-g1" "--graph1", required=True, help ="first graph file")
    parser.add_argument("-g2" "--graph2", required=True, help ="second graph file")
    parser.add_argument("-s" "--sim", required=True, help ="sim file")
    parser.add_argument("-d" "--delta", required=True, help ="delta value")
    parser.add_argument("-r" "--runs", required=False, default = 1, help = "number of times to run")
    parser.add_argument("-pk" "--pickle", required=False, default = "", help = "location of existing pickle file")
    return parser
||||||| merged common ancestors
	parser = argparse.ArgumentParser()
	parser.add_argument("-g1" "--graph1", required=True, help ="first graph file")
	parser.add_argument("-g2" "--graph2", required=True, help ="second graph file")
	parser.add_argument("-s" "--sim", required=True, help ="sim file")
	parser.add_argument("-d" "--delta", required=True, help ="delta value")
	parser.add_argument("-o" "--output", required=False, help ="output filename")
	return parser
=======
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
>>>>>>> 9d46a06f71c587b60383c55831059bd6b7c8ed73


'''
def convertSim(sim_file,d1,d2):
<<<<<<< HEAD
    f = open("convertedSim.txt", 'w')
    for line in lzma.open(sim_file, mode = 'rt'):
        n1, n2, s = line.split()
        try:
            f.write(f"{d1[n1]} {d2[n2]} {s}\n")
        except:
            pass
    f.close()
'''
||||||| merged common ancestors
	f = open("convertedSim.txt", 'w')
	for line in lzma.open(sim_file, mode = 'rt'):
		n1, n2, s = line.split()
		try:
			f.write(f"{d1[n1]} {d2[n2]} {s}\n")
		except:
			pass
	f.close()

=======
    f = open("convertedSim.txt", 'w')
    for line in lzma.open(sim_file, mode = 'rt'):
        n1, n2, s = line.split()
        try:
            f.write(f"{d1[n1]} {d2[n2]} {s}\n")
        except:
            pass
    f.close()
'''

def get_g1_seed(g1_seed_file) -> dict:
    g1_seed = defaultdict(list)
    for line in open(g1_seed_file):
        line = line.split()
        g1_seed[line[0]].append(line[1:])
    return g1_seed

def generate_seed(g1_seed_file, g2_seed_file):
    g1_seed = get_g1_seed(g1_seed_file)
    print (g1_seed)
    for line in open(g2_seed_file):
        line = line.split()
        if line[0] in g1_seed:
            for nodes in g1_seed[line[0]]:
                print (nodes, line[1:])
                yield nodes,line[1:]


>>>>>>> 9d46a06f71c587b60383c55831059bd6b7c8ed73

if __name__ == '__main__':
<<<<<<< HEAD
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

||||||| merged common ancestors
	parser = initParser()
	args = parser.parse_args()

	#g1 = build_graph("/home/sana/networks/yeast-reduced-wayne.el")

	
	
	#print(args.g1__graph1)
	#print(args.g2__graph2)
	#print(args.s__sim)

	delta = float(args.d__delta)

	graph1 = build_graph(args.g1__graph1)
	graph1.name = os.path.basename(args.g1__graph1)
	graph1.name = os.path.splitext(graph1.name)[0]
	graph2 = build_graph(args.g2__graph2)
	graph2.name = os.path.basename(args.g2__graph2)
	graph2.name = os.path.splitext(graph2.name)[0]
	#print("lengths for g1 and g2: ", len(graph1), len(graph2))

	sims = get_sim(args.s__sim, graph1, graph2)

	for i in range(10):
		start = time.time()
		a, b, pairs = dijkstra(graph1, graph2, get_seed(args.s__sim, graph1, graph2), sims, delta)
		subgraph = induced_subgraph(graph1, graph2, list(pairs))
		cov = max(coverage(graph1, graph2, subgraph))
		t = time.time() - start
		fname = graph1.name + "-" + graph2.name + '-' + str(delta) + f'-{cov:.2f}-'+ f'{t:.2f}' +".align"
		print(fname)
		write_result(fname, pairs, graph1, graph2)
=======
    parser = initParser()
    args = parser.parse_args()

    delta = float(args.delta)

    graph1 = build_graph(args.graph1)
    graph1.name = os.path.basename(args.graph1)
    graph1.name = os.path.splitext(graph1.name)[0]
    graph2 = build_graph(args.graph2)
    graph2.name = os.path.basename(args.graph2)
    graph2.name = os.path.splitext(graph2.name)[0]
    
#    runs = int(args.runs)

#    print(runs)
    
    #sims = get_sim(args.sim, graph1, graph2, args.pickle)
    #fname = graph1.name + "--" + graph2.name + "--" + str(delta)
    #print(fname)

    g1_seed_file = args.g1seed
    g2_seed_file = args.g2seed
	
    print (len(graph1), len(graph2))

    sims = degree_diff_matrix(graph1, graph2)
    for seed in generate_seed(g1_seed_file,g2_seed_file):
        a, b, pairs = dijkstra(graph1, graph2, get_aligned_seed(zip(*seed), graph1, graph2), sims, delta)
        subgraph = induced_subgraph(graph1, graph2, list(pairs))
        cov = coverage(graph1, graph2, subgraph)[0]
        print(cov)
        #print(subgraph)
    

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
>>>>>>> 9d46a06f71c587b60383c55831059bd6b7c8ed73
