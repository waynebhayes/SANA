from collections import defaultdict
def adj_mat(nodes, graph):
    print('nodes:',nodes)
    mat = [[0]*len(nodes) for _ in range(len(nodes))]
    for i in range(len(mat)):
        for j in range(i, len(mat)):
            if graph.has_edge(graph.indexes[nodes[i]],graph.indexes[nodes[j]]):
                mat[i][j] = 1
                mat[j][i] = 1
    return mat




def get_g1_seed(g1_seed_file) -> dict:
    g1_seed = defaultdict(list)
    for line in open(g1_seed_file):
        line = line.split()
        g1_seed[line[0]].append(line[1:])
    return g1_seed




def generate_seed(g1_seed_file, g2_seed_file):
    g1_seed = get_g1_seed(g1_seed_file)
    #print (g1_seed)
    for line in open(g2_seed_file):
        line = line.split()
        if line[0] in g1_seed:
            for nodes in g1_seed[line[0]]:
                yield nodes,line[1:]


def get_seed_length(network) -> int:
    #RNorvegicus_5_30_300000_MAX.txt 
    info = network.split("/")
    data = info[-1].split("_")
    return data[1]

