#!/bin/bash
#SBATCH --job-name=sana-PPT-3.3-20
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=20
#SBATCH --nodelist=cirnicus-11
#SBATCH --output=/home/lmacadar/Desktop/Research/SANA/scripts/parallel-performance-testing/output/3.3-20-cirnicus-11-run0.out
#SBATCH --error=/home/lmacadar/Desktop/Research/SANA/scripts/parallel-performance-testing/output/3.3-20-cirnicus-11-run0.out

cd /home/lmacadar/Desktop/Research/SANA
SANA_THREADS="20" /usr/bin/time -f 'TIMING: real=%e user=%U sys=%S' ./sana3.3 -g1 yeast -g2 human -ec 1 -tolerance 0 -it 100000000 -tinitial 0.01 -tdecay 6 -maxthreads 20
