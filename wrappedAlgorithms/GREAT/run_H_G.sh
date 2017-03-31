
#gets both networks and alpha parameter from user
networkA="$1"
networkB="$2"
shift 2
# alphas are on remainder of command line.

for i in "$@"
do
    alpha=$i

    #makes the directories
    mkdir "H-G"
    mkdir "H-G/"$alpha
    dir="H-G/"$alpha"/"$networkA
    dest_time=$dir"/"$networkA"_times.txt"
    mkdir $dir
    dir="H-G/"$alpha"/"$networkA"/"$networkB"/"
    mkdir $dir



    gwA="networks/"$networkA".gw"
    gwB="networks/"$networkB".gw"
    countA="orbits/"$networkA"_count"
    countB="orbits/"$networkB"_count"

    scores=$dir$networkA"_"$networkB
    egdcA=$dir$networkA".egdc"
    egdcB=$dir$networkB".egdc"
    countsA=$countA".ecounts"
    countsB=$countB".ecounts"
    time=$scores"_time.txt"

    #gets graphlet degree centrality score for each network
    /usr/bin/time -f "%S   %U" -o $time -a src/egdc $countsA $egdcA
    /usr/bin/time -f "%S   %U" -o $time -a src/egdc $countsB $egdcB

    #gets the graphlet degree vector signature
    egdvs=$dir$networkA"_"$networkB".egdvs"
    /usr/bin/time -f "%S   %U" -o $time -a src/egdvs $countsA $countsB $egdvs

    #combines the gdvs and gdc scores
    /usr/bin/time -f "%S   %U" -o $time -a python src/combiner.py $egdcA $egdcB $egdvs $alpha

    ealn=$scores".ealn"
    aln=$scores".aln"
    param=$scores"_"$alpha"_scores_hung.txt"
    param2=$scores"_"$alpha"_scores.txt"
    cealn=$scores"_conv.ealn"
    caln=$scores"_hung_final.aln"
    matrix=$scores"_matrix_alignment_hung.aln"


    label=$scores"_node_label.txt"
    node_score=$scores"_node_score.txt"
    node_score_greedy=$scores"_node_score_greedy.txt"

    #plugs the distance matrix into the hungarian function
    /usr/bin/time -f "%S   %U" -o $time -a src/find_one_opt_align $ealn $matrix $param

    #converts output of the Hungarian function to the format used by the node similarity function
    /usr/bin/time -f "%S   %U" -o $time -a python src/hung_conv.py $ealn $param2 $cealn

    #gets the node similairty score based on the edge alignment
    /usr/bin/time -f "%S   %U" -o $time -a python src/ealign2nscore_sum.py $cealn $egdvs $node_score $label

    #converts score so that it is usable by the Hungarian function
    /usr/bin/time -f "%S   %U" -o $time -a python src/num_conv.py $node_score

    # Uses node similarity scores to create the node mapping
    /usr/bin/time -f "%S   %U" -o $time -a python src/greedy_matrix.py $node_score_greedy $matrix

    # Outputs the node mapping with node labels
    /usr/bin/time -f "%S   %U" -o $time -a python src/alignment.py $matrix $label $aln

    /usr/bin/time -f "%S   %U" -o $time -a python src/isolated_nodes.py $aln $gwA $gwB

    net=$networkA"_"$networkB
    python src/time.py $time $dest_time $net

    dir="H-G/"$alpha"/"$networkA"/Alignments"
    mkdir $dir

    cp $aln $dir
    rm $param
    rm $node_score_greedy
    rm $matrix
    rm $egdvs
    rm $egdcA
    rm $egdcB
    rm $migraal
    rm $node_score_hung
    rm $param2
    rm $node_score
done





