#made by Vikram Saraph

from sys import argv

goterms1 = open(argv[1])
goterms2 = open(argv[2])
aln = open(argv[3])
sim = open(argv[4])
out = open(argv[5], 'w')

# Read GO term associations
genegoterms1 = {}
for line in goterms1:
	tokens = line.strip().split()
	gene = tokens[0]
	term = tokens[1]
	if gene in genegoterms1:
		genegoterms1[gene].add(term)
	else:
		genegoterms1[gene] = set([term])

# And again
genegoterms2 = {}
for line in goterms2:
	tokens = line.strip().split()
	gene = tokens[0]
	term = tokens[1]
	if gene in genegoterms2:
		genegoterms2[gene].add(term)
	else:
		genegoterms2[gene] = set([term])

# And read GO term pairwise similarities
gotermsim = {}
for line in sim:
	tokens = line.strip().split()
	pair = (tokens[0], tokens[1])
	try:
		gotermsim[pair] = float(tokens[2])
	except ValueError:
		gotermsim[pair] = 0

count = 0
total = 0.
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
	simsum = 0
	npairs = len(terms1) * len(terms2)
	for t1 in terms1:
		for t2 in terms2:
			simsum += gotermsim[(t1, t2)]
	try:
		proteinpairsim = float(simsum) / npairs
	except ZeroDivisionError:
		proteinpairsim = float('-inf')
	if proteinpairsim != float('-inf'):
		total += proteinpairsim
		count += 1
	out.write("%s\t%s\t%f\n" % (pair[0], pair[1], proteinpairsim))
print total / count
out.close()
