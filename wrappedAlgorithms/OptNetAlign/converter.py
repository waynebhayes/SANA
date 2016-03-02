if __name__ == "__main__":
    Input = input("Enter .gw file path: ")
    file = open(Input, 'r')
    output = open(Input.rstrip(".gw") + ".txt", 'w')
    for f in file.readlines():
        line = f.split()
        try:
            eval(line[0])
        except:
            pass
        else:
            try:
                output.write(line[0] + " " + line[1] + "\n")
            except IndexError:
                pass