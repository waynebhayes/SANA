from collections import defaultdict
import sys
if __name__ == "__main__":
    file_name = sys.argv[1]
    file = open(file_name, 'r')
    out_file_name = file_name.rstrip('.gw') + '.gml'
    out = open(out_file_name, 'w')
    Output = ''
    Output += "graph\n[\n"
    node_list = []
    i = 0
    edges = defaultdict(list)
    for line in file.readlines():
        line_list = line.split()
        #print(line_list)
        try:
            eval(line_list[0])
        except Exception as e:
            if e == IndexError:
                break
            if line_list[0][0] == '|':
                node = line_list[0].rstrip('}|')
                node = node.lstrip('|{')
                node_list.append(node)
                output = "\tnode\n\t[\n\tid {l}\n\tlabel \"{n}\"\n\tindex {l}\n\t]\n".format(n = node, l = i)
                i += 1
                Output += output
        else:
            #print(len(node_list))
            if len(line_list) > 1:
                edges[eval(line_list[0])-1].append(eval(line_list[1])-1)
            
    for key in sorted(edges):
        for second_key in sorted(edges[key]):        
            output = "\tedge\n\t[\n\tsource {a}\n\ttarget {b}\n\t]\n".format(a = key, b = second_key)
            Output += output
    Output += ']'
    out.write(Output)
