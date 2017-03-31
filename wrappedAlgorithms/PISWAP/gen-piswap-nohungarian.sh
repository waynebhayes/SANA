#!/bin/sh
die(){ echo "$@" >&2; exit 1;
}
[ -f "$1" ] || die "no such file '$1'"

cat << EOF
from __future__ import print_function
import PSB2009_3opt_2 as psb09
import matching as match
import sys

if len(sys.argv) < 5:
	print("Need three input files and a double alpha: g1, g2, sim, alpha")
else:
	G = psb09.getGraph(sys.argv[1]) #input network1
	G2 = psb09.getGraph(sys.argv[2])#input network2
	GS = psb09.graphScores(sys.argv[3])
	alpha = float(sys.argv[4])

EOF

./align2match.sh "$1"

cat << EOF
	(S, M) = psb09.processOnce(G, G2, GS, M0, alpha, 200) #run PISwap

	#output the alignment result
	F = open("match_output.txt","w")
	for node in M:
	 print(node+" "+M[node], file = F)
	F.close()
EOF
