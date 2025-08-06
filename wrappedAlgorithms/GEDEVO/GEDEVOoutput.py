import sys

with open(sys.argv[1], 'r') as alignment:
    comments = True
    alignments = []
    for line in alignment:
        if comments:
            if line[:2] != "##":
                comments = False
        else:
            alignedNodes = line.split()[:2]
            if alignedNodes[0] != '-' and alignedNodes[1] != '-':
                alignments.append((alignedNodes[1], alignedNodes[0]))
                    
with open(sys.argv[2], 'w') as output:
    for alignment in alignments:
        output.write(alignment[0] + "\t" + alignment[1] + "\n")