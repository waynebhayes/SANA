PINALOG 1.0 README

===================
PINALOG 1.0 has been developed by Hang Phan, PhD student at the Division of Molecular Biosciences, Imperial College London, SW7 2AZ
For any query concerning PINALOG, please contact hang.phan07@imperial.ac.uk

PINALOG 1.0 is executable on 32 bit Linux system due to the requirement of the community detection software CFinder. For use on 64 bit system,
please contact http://www.cfinder.org/ for a license and hang.phan07@imperial.ac.uk for the compatible PINALOG1.0 version. 
===================

1. pinalogog1.0.tar.gz file content

   1. PINALOG1.0 executable file
   2. CFinder executable file for detecting communities in the network
   3. Gene ontology file (when using for function annotation), go to the GO consortium for the latest version
   4. Gene associations of proteins in the IntAct database, extracted from the GOA set, used as background for calculating functional similarity
   5. Sample mouse and rat PPI network with associated BLAST data and gene annotation data


2. Preparing the input
   
   2.1 Network file format
       Tab-delimitted file containing two columnms
       ID1 ID2
       ID3 ID2
       ...
       where ID1, ID2, ID3 are the string identifying the protein names in the species 

   2.2. BLAST data file:
      This file contains the result of the all-against-all BLAST results of proteins in the input species. 
      This includes the BLAST results of the proteins within each species as well as with those in the other species. 
      The self-BLAST score of each protein is required so that a normalization on the sequence similarity score to be accurately computed.
      The format of the BLAST data file is as below:

	   ID1 ID2 BLAST-score
      
       where ID1, ID2 are protein IDs of the input species, and BLAST-score is the score obtained from the BLAST sequence alignment result.

   2.3 GO annotation file format:
      The annotation file has the format following the GO consortium file format 

3. Running PINALOG1.0
   
   3.1 Using only sequence information in alignment

       > ./pinalog1.0 network1 network2 blastdata
      e.g. 
       > ./pinalog1.0 mouse.pin rat.pin mouse_rat.blast_score
   
   3.2 Using also protein function in the alignment process 

       > ./pinalog1.0 network1 network2 blastdata gene_association
      e.g
       > ./pinalog1.0 mouse.pin rat.pin mouse_rat_blast_score mouse_rat.gene_association

  Please be advised that when including the function similarity in the alignment process, the time spent to calculate the function similarity matrix is long. 5hrs is expected for large PPIs (~5000 - ~9000 proteins).
  
4. Output

    4.1 Aligned proteins: are stored in net1_net2.pinalog.nodes_algn, first column is species1, second column is species2 and third column is the combined sequence and function similarity of the mapped protein pairs. 
    4.2 Conserved interactions: are stored in net1_net2.pinalog.edges_algn, each line has 2 columns
      
       P1_Q1 P2_Q2

   where P1 in species 1 is mapped to Q1 in species 2, P2 in species 1 is mapped to Q2 in species 2 and there is an edge P1-P2 in PPI1 as well as an edge Q1-Q2 in PPI2
    
