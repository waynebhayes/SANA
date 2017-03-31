Last updated in Feb. 14, 2008

Please note that the depcrecated FTP file formats (hmlg.ftp and hmlg.trip.ftp) 
will be discontinued as of Jan. 1, 2007.

HomoloGene has adopted new formats for its FTP files.  Every build
will contain the following files:

homologene.data
homologene.xml.gz

build_inputs/all_proteins.data
build_inputs/proteins_for_clustering.data
build_inputs/taxid_taxname

----------------------------------------------------------
homologene.data is a tab delimited file containing the following
columns:

1) HID (HomoloGene group id)
2) Taxonomy ID
3) Gene ID
4) Gene Symbol
5) Protein gi
6) Protein accession
-----------------------------------------------------------
homologene.xml.gz is a gzip'ed file containing a XML dump of the
HomoloGene build.  It contains much more information than
homologene.data (such as gene and protein links, distance analysis results,
etc). Please note that an improved procedure for updating PubMed publications 
corresponding to HomoloGene groups has been adopted, to keep this information 
up-to-date. Therefore, publication information is no longer included in the file 
homologene.xml.gz, but are available on the HomoloGene web site.  
The format of homologene.xml.gz is explanined in the NCBI DTD specifications,  
that can be downloaded at: 
http://www.ncbi.nlm.nih.gov/data_specs/dtd/  


The old HomoloGene FTP file formats (hmlg.ftp and hmlg.trip.ftp) are now
deprecated.  Please refer to README-old if you need a description
of the old format files.  Due to the nature of the new build procedure,
a new type of hit data has been added: m signifies that there is a
match between the two sequences, although they might not necessarily
be reciprocal best.  Thus hits in hmlg.ftp can be one of the following:
b - reciprocal best, B - reciprocal best in a self-consistent triplet,
m - similarity between sequences.

----------------------------------------------------------------
build_inupts/taxid_taxname  

is a tab delimited file containing the following columns:

1) taxonomy id
2) taxonomic name

----------------------------------------------------------------
build_inputs/all_proteins.data 

is a tab delimited file that lists all proteins and their gene information. 
If a gene has multple protein accessions derived from alternative splicing event, 
each protein accession is list in a separate line. 

tab-delimited columns are:

1) taxid
2) entrez GeneID
3) gene symbol
4) gene description
5) protein accession.ver
6) mrna accession.ver
7) length of protein  listed in column 5)
8) -11) contains data about gene location on the genome
8) nucleotide gi of genomic sequence where this gene is annotated 
9) starting position of gene in 0-based coordinate
10) end position of the gene in 0-based coordinate 
11) strand



----------------------------------------------------------------
build_inputs/proteins_for_clustering.data

is a tab delimited file that lists one protein per gene that were used for homologene clustering. 
If a gene has multiple protein accessions derived from alternative splicing, 
only one protein isoform that give most protein alignment to proteins in other species 
was selected for clustering and it is listed in this file. 
Its format is the same as all_proteins.data and contains following columns:

1) taxid
2) entrez GeneID
3) gene symbol
4) gene description
5) protein accession.ver
6) mrna accession.ver
7) length of protein  listed in column 5)
8) -11) contains data about gene location on the genome
8) nucleotide gi of genomic sequence where this gene is annotated 
9) starting position of gene in 0-based coordinate
10) end position of the gene in 0-based coordinate 
11) strand

--------------------------------------------------------------------



