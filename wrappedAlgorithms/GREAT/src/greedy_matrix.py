import sys
import random
import mmap

random.seed()
outf = open(sys.argv[2], 'w')
with open(sys.argv[1], 'r') as f:
    infile = mmap.mmap(f.fileno(),0,prot=mmap.PROT_READ)

infile.readline()
matrix = []

for l in iter(infile.readline, ""):
    terms = l.split()
    row = []
    for t in terms:
        row.append(float(t))
    matrix.append(row)
    
good_rows = range(0, len(matrix))
good_cols = range(0, len(matrix[0]))
aligned_pairs = []

infile.close()

print "loaded"

values = []
for i in good_rows:
    for j in good_cols:
        values.append((matrix[i][j],i,j))

random.shuffle(values)        
values.sort(key = lambda x: x[0])
values.reverse()

for val,r, c in values:
     if(r in good_rows and c in good_cols):
        aligned_pairs.append((r, c, val))
        good_rows.remove(r)
        good_cols.remove(c)
        if(len(good_rows) % 100 == 0):
            print 100*(float(len(matrix)) -len(good_rows))/len(matrix), "%"
    
   
    


for p in aligned_pairs:
    outf.write(str(p[0]+1) + "\t" + str(p[1]+1) + "\t" + str(p[2])+ "\n")
    
outf.close()
    
