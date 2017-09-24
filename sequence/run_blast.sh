#!/usr/bin/env bash

args=" -outfmt 6 -word_size 3 -evalue 100" #faster version
args=" -outfmt 6 -word_size 2 -evalue 1000 -num_threads 2 "

function run_blast { # ntework, db
    local network=$1; shift

    for db in $@
    do
    echo "G1: ${network} G2: ${db}"
    ./blast/makeblastdb -in FASTA/${db}.fasta -dbtype prot -out ${db}
	time ./blast/blastp -query FASTA/${network}.fasta -db ${db} -out scores/${network}_${db}_blast.out ${args}
	rm ${db}.{phr,pin,psq}
	done

}

#run_blast RNorvegicus SPombe CElegans MMusculus AThaliana DMelanogaster
#run_blast SPombe CElegans MMusculus AThaliana DMelanogaster
#run_blast CElegans MMusculus AThaliana DMelanogaster
#run_blast MMusculus AThaliana DMelanogaster
#run_blast AThaliana DMelanogaster
run_blast $1 $2

