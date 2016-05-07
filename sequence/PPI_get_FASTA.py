#!/python
import sys
import operator
import Bio.Entrez as BE


# load entrez-ids from a PPI (in leda format)
def readPPI(fileName):
	ofile = open(fileName, "r")
	vertices = []
	nb_vertex = 0
	ofile.readline()
	ofile.readline()
	ofile.readline()
	ofile.readline()
	line = ofile.readline()
	nb_vertex = int(line.split()[0])
	for i in range(nb_vertex):
		line = ofile.readline()
		vertex = line.split()[0][2:-2]
		vertices.append(vertex)
	
	ofile.close()
	return vertices



def fasta_from_entrezid(fileName, entrez_ids):
	ofile = open(fileName, "w")
	count = 1
	for eid in entrez_ids:
		print 'processing ',count, ' / ', len(entrez_ids)
		try:
			res = BE.elink(db='protein', dbfrom='gene', id=eid,retmode='text').read().split()
			mylst = []
			for line in res:
				if line.split('>')[0] == '<Id':
					rid = line.split('>')[1].split('<')[0]
					mylst.append(rid)
			#pid = res[0]['LinkSetDb'][0]['Link'][0]['Id']
			pid = mylst[1]
			handle = BE.efetch(db='protein', id=pid, rettype='fasta', retmode='text')
			pseq = handle.read()
			seq = pseq.split('\n')
			if seq > 1:
				ofile.write(">%s\n"%(eid))
				for i in range(1,len(seq)):
					ofile.write("%s\n"%(seq[i]))
		except:
			print "seq: %s not found"%(eid)
		count=count+1
	ofile.close()
	

BE.email = 'set your email here'

# Read the publications and experiments types in the database for human-human interactions
PPI_File = sys.argv[1] #  <-- input .gw leda PPI network
Fasta_File = sys.argv[2] #  <-- output .fasta file

print "## Loading entrez-ids from PPI file\n"
entrez_list = readPPI(PPI_File)

# Remove the ubiquitin protein as it distorts the topology

print "## Retrieving FASTA sequences\n"
nodes = fasta_from_entrezid(Fasta_File, entrez_list)



