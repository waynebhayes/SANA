import fileinput
import sys

for line in fileinput.input(sys.argv[1], inplace=True): 
      print line.replace('-', '='),





