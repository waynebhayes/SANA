import sys

outf = open(sys.argv[3], 'w')
out_label = open(sys.argv[4],'w')
infile = open(sys.argv[1], 'r')
labelfile = open(sys.argv[2], 'r')

header = labelfile.readline()
headers2 = header.split()
headers1 = []


for l in labelfile.readlines():
    headers1.append(l.split()[0])


nodes1 = [x.split("-")[0] for x in headers1] + [x.split("-")[1] for x in headers1]
nodes1 = list(set(nodes1))

nodes2 = [x.split("-")[0] for x in headers2] + [x.split("-")[1] for x in headers2]
nodes2 = list(set(nodes2))
print len(nodes1), len(nodes2)


matrix = {}
for n1 in nodes1:
    matrix[n1] = {}
    for n2 in nodes2:
        matrix[n1][n2] = 0

for l in infile.readlines():
    terms = l.split()
    edge1 = headers1[int(terms[0])-1]
    edge2 = headers2[int(terms[1])-1]
    n1 = edge1.split("-")[0]
    n2 = edge1.split("-")[1]
    n3 = edge2.split("-")[0]
    n4 = edge2.split("-")[1]
    val = float(terms[2])
    
    matrix[n1][n3] += 1
    matrix[n2][n4] += 1
    matrix[n1][n4] += 1
    matrix[n2][n3] += 1



outf.write(str(len(nodes1)) + "\t" + str(len(nodes2)))

   
for n1 in nodes1:
    outf.write("\n")
    for n2 in nodes2:
        outf.write(str(matrix[n1][n2])+"\t")
outf.close()

for n2 in nodes2:
    out_label.write(" " + n2)
out_label.write("\n")

for n1 in nodes1:
    out_label.write(" " + n1)

out_label.close()
        
    
    
    






    
    
    




