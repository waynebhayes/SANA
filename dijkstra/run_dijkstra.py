import argparse
#from research_for_graph import *
from graph_research import *
import lzma
import time
import os.path

def initParser():
	parser = argparse.ArgumentParser()
	parser.add_argument("-g1" "--graph1", required=True, help ="first graph file")
	parser.add_argument("-g2" "--graph2", required=True, help ="second graph file")
	parser.add_argument("-s" "--sim", required=True, help ="sim file")
	parser.add_argument("-d" "--delta", required=True, help ="delta value")
	parser.add_argument("-o" "--output", required=False, help ="output filename")
	return parser


def convertSim(sim_file,d1,d2):
	f = open("convertedSim.txt", 'w')
	for line in lzma.open(sim_file, mode = 'rt'):
		n1, n2, s = line.split()
		try:
			f.write(f"{d1[n1]} {d2[n2]} {s}\n")
		except:
			pass
	f.close()


if __name__ == '__main__':
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
