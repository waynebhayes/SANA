#!/usr/bin/env bash
# scrit that executes PPI_get_FASTA.py for the networks in the BioGRID dataset
python PPI_get_FASTA.py ../networks/RNorvegicus/RNorvegicus.gw networks/RNorvegicus/go/RNorvegicus.fasta &
python PPI_get_FASTA.py ../networks/SPombe/SPombe.gw networks/SPombe/go/SPombe.fasta &
python PPI_get_FASTA.py ../networks/CElegans/CElegans.gw networks/CElegans/go/CElegans.fasta &
python PPI_get_FASTA.py ../networks/MMusculus/MMusculus.gw networks/MMusculus/go/MMusculus.fasta &
python PPI_get_FASTA.py ../networks/AThaliana/AThaliana.gw networks/AThaliana/go/AThaliana.fasta &
python PPI_get_FASTA.py ../networks/DMelanogaster/DMelanogaster.gw networks/DMelanogaster/go/DMelanogaster.fasta &
