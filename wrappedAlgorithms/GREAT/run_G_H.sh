
# Gets both networks and alpha parameter form user
networkA="$1"
networkB="$2"
shift 2
# alphas are on remainder of command line.

for i in "$@"
do
    alpha=$i
    #makes the directories
    mkdir "G-H"
    mkdir "G-H/"$alpha
    dir="G-H/"$alpha"/"$networkA"/"
    dest_time=$dir"/"$networkA"_times.txt"
    mkdir $dir
    dir="G-H/"$alpha"/"$networkA"/"$networkB"/"
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

    # Gets the graphlet degree centrality score from both networks
    /usr/bin/time -f "%S   %U" -o $time -a src/egdc $countsA $egdcA
    /usr/bin/time -f "%S   %U" -o $time -a src/egdc $countsB $egdcB

    # Gets teh graphlet degree vector similarity score`
    egdvs=$dir$networkA"_"$networkB".egdvs"
    /usr/bin/time -f "%S   %U" -o $time -a src/egdvs $countsA $countsB $egdvs



    # Combines the gdc and gdvs scores
    /usr/bin/time -f "%S   %U" -o $time -a python src/combiner.py $egdcA $egdcB $egdvs $alpha

    ealn=$scores".ealn"
    aln=$scores".aln"
    param=$scores"_"$alpha"_scores.txt"
    cealn=$scores"_conv.ealn"
    caln=$scores"_hung_final.aln"
    matrix=$scores"_matrix_alignment_hung.aln"

    label=$scores"_node_label.txt"
    node_score=$scores"_node_score.txt"
    node_score_hung=$scores"_node_score_hung.txt"
    matrix=$scores"_matrix_alignment.aln"

    # Plugs matrix of scores into the greedy function, to get an edge alignment
    /usr/bin/time -f "%S   %U" -o $time -a python src/greedy_matrix.py $param $ealn

    # Use edge alignment to get node similarity scores
    /usr/bin/time -f "%S   %U" -o $time -a python src/ealign2nscore_sum.py $ealn $egdvs $node_score $label

    # Scales the values to be between 0 and 1
    /usr/bin/time -f "%S   %U" -o $time -a python src/num_conv.py $node_score

    #gets the final alignments
    /usr/bin/time -f "%S   %U" -o $time -a src/find_one_opt_align $aln $matrix $node_score_hung

    #converts the output of the Hungarian function so that we know the node mappings by name
    /usr/bin/time -f "%S   %U" -o $time -a python src/hung_align.py $aln $label $caln

    /usr/bin/time -f "%S   %U" -o $time -a python src/isolated_nodes.py $caln $gwA $gwB

    net=$networkA"_"$networkB
    python src/time.py $time $dest_time $net

    dir="G-H/"$alpha"/"$networkA"/Alignments"
    mkdir $dir
    paramh=$scores"_"$alpha"_scores_hung.txt"

    cp $caln $dir
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
    rm $paramh
done











