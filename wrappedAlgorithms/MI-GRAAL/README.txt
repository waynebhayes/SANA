From: Viet Ly

Dear Professor Hayes,

Here is a quick guide on how to use MIGRAAL in SANA. 

There are three arguments you can pass to SANA via -wrappedArgs:
-q     Sequence similarity file
-p     Specify Cost Matrices
-s     Seed

The format for the sequence similarity file is:
node_1 node_2 score (each column is separated by a space)

For the -p parameter, a single number specifies the cost matrices to use. The following cost measures are: 

1 Signature Similarites
2 Degrees
4 Clustering coefficients
8 eccentricities
16 Sequence scores, or any other matrix, using parameter -q
32 Betweeness centralitites.

-s parameter specifies what number to seed the random number generator with

Example command: ./sana -method MIGRAAL -wrappedArgs '-p 19 -q seq_sim_file'
This command will instruct MIGRAAL to use the following cost measures: sequence similarities (1), degrees (2), and sequence scores (16). The sequence file, specified with -q, is seq_sim_file.

Please let me know if you have any questions regarding running MIGRAAL in SANA.
