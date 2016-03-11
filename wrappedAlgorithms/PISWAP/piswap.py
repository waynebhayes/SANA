from __future__ import print_function
import PSB2009_3opt_2 as psb09
import matching as match
import sys

if len(sys.argv) < 3:
	print("Need two input files")
else:
	G = psb09.getGraph(sys.argv[1]) #input network1
	G2 = psb09.getGraph(sys.argv[2])#input network2
	GS = psb09.graphScores("pairwise_sequence_similarity_of_network1_and_2.evals")

	#input pairwise sequence similarity of network1 and 2.
	M0 = match.max_weight_matching(GS) #run hungorian algorithm to produce initial alignment
	(S, M) = psb09.processOnce(G, G2, M0, 0.6, 200) #run PISwap

	#output the alignment result
	F = open("match_output.txt","w")
	for node in M:
	 print(node+" "+M[node], file = F)
	F.close()
