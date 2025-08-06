import sys

fileHeader = sys.argv[1][:-3]
    
with open(sys.argv[1], 'r') as GWfile:
    lineCount = 0
    nodeCount = 0
    nodes = {}
    interactions = []
    for line in GWfile:
	print(line)
        if lineCount == 4:
            nodeCount = int(line)
        elif lineCount > 4 and lineCount <= 4 + nodeCount:
            currentNode = line[2:-3]
            nodes[lineCount-4] = currentNode
        elif lineCount > 4 + nodeCount + 1:
            interactingEdges = line.split()[0:2]
            interactions.append((int(interactingEdges[0]), int(interactingEdges[1])))
            
        lineCount += 1
    
with open(sys.argv[2], 'w') as NTWfile:
    NTWfile.write("code\tch1\tch2\tN\tlength1\tlength2\tidentical\thomolog\thomomer\tstruct1\tstruct2\n")
    for interaction in interactions:
        NTWfile.write(fileHeader + "\t" + nodes[interaction[0]] + "\t" + nodes[interaction[1]] + "\t")
        NTWfile.write("-\t-\t-\t-\t-\t-\t-\t-\n")
