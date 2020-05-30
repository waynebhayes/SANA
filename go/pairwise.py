#made by Vikram Saraph

from sys import argv

goterms1 = open(argv[1])
goterms2 = open(argv[2])
aln = open(argv[3])
out = open(argv[4], 'w')

genegoterms1 = {}
for line in goterms1:
	tokens = line.strip().split()
	gene = tokens[0]
	term = tokens[1]
	if gene in genegoterms1:
		genegoterms1[gene].add(term)
	else:
		genegoterms1[gene] = set([term])

genegoterms2 = {}
for line in goterms2:
	tokens = line.strip().split()
	gene = tokens[0]
	term = tokens[1]
	if gene in genegoterms2:
		genegoterms2[gene].add(term)
	else:
		genegoterms2[gene] = set([term])

termpairs = set()
for line in aln:
	pair = line.strip().split()
	try:
		terms1 = genegoterms1[pair[0]]
	except KeyError:
		terms1 = []
	try:
		terms2 = genegoterms2[pair[1]]
	except KeyError:
		terms2 = []
	for t1 in terms1:
		for t2 in terms2:
			termpairs.add((t1, t2))

for pair in termpairs:
	out.write("%s\t%s\n" % (pair[0], pair[1]))
out.close()
