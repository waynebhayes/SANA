import sys

def convertOutput(inPath, outPath):
    inFile = open(inPath, 'r')
    outFile = open(outPath, 'w')
    lines = inFile.readlines()
    inFile.close()
    counter = 0
    for line in lines:
        splitLine = line.split()
        outFile.write(splitLine[0] + '\t' + splitLine[1])
        if counter != len(lines)-1:
            outFile.write('\n')
        counter = counter+1
    outFile.close()

if __name__ == "__main__":
    convertOutput(sys.argv[1], sys.argv[2])
