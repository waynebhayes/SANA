import sys
if __name__ == "__main__":
    output = sys.argv[1]
    in_file = open(output, 'r')
    out_string = ""
    for i in range(2):
        in_file.readline()
    for k in in_file.readlines():
        line = k.split()
        out_string += line[0] + " " + line[1] + "\n"
    out_name = output.rstrip("txt") + "aln"
    out_file = open(out_name, 'w')
    out_file.write(out_string)
