#!/bin/sh
die(){ echo "$@" >&2; exit 1;
}
[ -f "$1" ] || die "no such file '$1'"

cat << EOF
from __future__ import print_function
import PSB2009_3opt_2 as psb09
import matching as match
import sys

if len(sys.argv) < 4:
	print("Need three input files: g1, g2, sim")
else:
	G = psb09.getGraph(sys.argv[1]) #input network1
	G2 = psb09.getGraph(sys.argv[2])#input network2
	GS = psb09.graphScores(sys.argv[3])

	#input pairwise sequence similarity of network1 and 2.
EOF

./align2match.sh "$1"

cat << EOF
	(S, M) = psb09.processOnce(G, G2, M0, 0.6, 200) #run PISwap

	#output the alignment result
	F = open("match_output.txt","w")
	for node in M:
	 print(node+" "+M[node], file = F)
	F.close()
EOF
