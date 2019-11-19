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

def get_aligned_seed(s, graph1, graph2):
    for pair in s:
        #yield [int(pair[0]),int(pair[1])]
        yield [graph1.indexes[pair[0]], graph2.indexes[pair[1]]]

def get_seed_length(network) -> int:
    #RNorvegicus_5_30_300000_MAX.txt 
    info = network.split("/")
    data = info[-1].split("_")
    return data[1]

def get_seed(file, graph1, graph2, delta):
    """
    get_seed() returns a generator that reads through a file and
    returns a 2-element tuple of yeast and human nodes. The file
    must be sorted in the order that seeds should be returned.
    In this case, it must be in descending order to find the
    highest similarity pair first
    file format: human_node yeast_node similarity
    type matching: int, int, float
    non-random version
        with open(file,'r') as f3:
            for line in f3:
                yield [int(n) for n in (line.strip().split()[0:2])]
    """
    tied_seeds = []
    #curr_value = 1.0
    curr_value = 1.0 - delta
    if file.endswith("xz"):
        with lzma.open(file, mode = 'rt') as f3:
            for line in f3:
                row = line.strip().split()
                if row[0] not in graph1.indexes or row[1] not in graph2.indexes:
                    continue
                row[0], row[1], row[2] = graph1.indexes[row[0]], graph2.indexes[row[1]], float(row[2])
                if row[2] < curr_value:
                    random.shuffle(tied_seeds)
                    for seed in tied_seeds:
                        yield seed[0:2]
                    del tied_seeds
                    tied_seeds = [row[0:2]]
                    curr_value = row[2]
                else:
                    tied_seeds.append(row[0:2])
            random.shuffle(tied_seeds)
            for seed in tied_seeds:
                yield seed[0:2]

    else:
        with open(file, mode = 'rt') as f3:
            for line in f3:
                row = line.strip().split()
                if row[0] not in graph1.indexes or row[1] not in graph2.indexes:
                    continue
                row[0], row[1], row[2] = graph1.indexes[row[0]], graph2.indexes[row[1]], float(row[2])
                if row[2] < curr_value:
                    random.shuffle(tied_seeds)
                    for seed in tied_seeds:
                        yield seed[0:2]
                    del tied_seeds
                    tied_seeds = [row[0:2]]
                    curr_value = row[2]
                else:
                    tied_seeds.append(row[0:2])
            random.shuffle(tied_seeds)
            for seed in tied_seeds:
                yield seed[0:2]


