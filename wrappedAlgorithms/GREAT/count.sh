gwA="networks/"$1".gw"
countA="orbits/"$1"_count"
python src/rename.py $gwA
src/ecount $gwA $countA
dumpA=$countA".ndump2"
ecountA="orbits/"$1"_count.ecounts"
python src/edge_counts_check.py $dumpA $ecountA $gwA
