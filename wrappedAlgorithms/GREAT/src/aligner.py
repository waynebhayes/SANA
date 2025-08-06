import sys

try:                                
    opts, args = getopt.getopt(argv, "hg:d", ["help", "grammar="]) 
except getopt.GetoptError:           
    usage()                          
    sys.exit(2)
    
if( len(args) != 3):
    print "Usage: " + args[0] + " [infile 1] [infile 2] [outfile]"
    sys.exit(2)
    
eGDC_file1 = open(args[1] + ".egdc", 'r')
eGDC_file2 = open(args[2] + ".egdc", 'r')