from builder import *
import numpy
##import random
from collections import defaultdict
import datetime
from skip_list import *
import lzma

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
                
def update_best_pair(pq, yeast_graph, human_graph, yeast_node, human_node, pairs, sims, delta = 0):
##    nonlocal pq
    paired_yeast_nodes = np.fromiter((pair[0] for pair in pairs), dtype=int)
    paired_human_nodes = np.fromiter((pair[1] for pair in pairs), dtype=int)
    yeast_neighbors = np.setdiff1d(
        yeast_graph.get_neighbors(yeast_node), paired_yeast_nodes)
    human_neighbors = np.setdiff1d(
        human_graph.get_neighbors(human_node), paired_human_nodes)
    # do the check here to save on a function call
    # also, saves on unnecessary enqueueing of empty pairs (-1, [])
    # cost: 2 extra comparisons and 1 boolean operation
##    print("yeast", yeast_neighbors)
##    print("human", human_neighbors)
    if yeast_neighbors.size == 0 or human_neighbors.size == 0:
        return
    
    bp_list = sub_best_pair(yeast_neighbors, human_neighbors, sims, delta)
    for (val, new_pairs) in bp_list:
        if val >= 0:
            for pair in new_pairs:
                pq.add((val, pair))
    
                ##pq.insert((val, pair))
    
##    (val, new_pairs) = sub_best_pair(yeast_neighbors, human_neighbors, sims)
####    print((val, new_pairs))
##    if val < 0:
##        return
##    pq.insert((val, new_pairs))

def sub_best_pair(yeast_neighbors, human_neighbors, sims, delta):
    """
    sub_best_pair takes takes two sets of yeast and human neighbors
    plus a similarity matrix and returns the highest similarity
    pair of nodes in the two sets of neighbor nodes.
    runtime: O(|Y| * |H|)
    """
    s = sims[np.ix_(yeast_neighbors,human_neighbors)]
    # If the matrix is empty, no pair is found
##    if s.size == 0:
##        return (-1,[])

    low = max(s.max() - delta, 0.0)
    answers = []
    while(s.max() >= low):
        y_found, h_found = np.where(s == s.max())
        node_pairs = np.column_stack((yeast_neighbors[y_found], human_neighbors[h_found]))
##    np.random.shuffle(nodes) #do the shuffle when popping instead
        answers.append((s.max(), node_pairs))
        s[y_found, h_found] = -1
    return answers

##    simple dfs is very bad
##    y = random.sample(yeast, 1)[0]
##    h = random.sample(human, 1)[0]
##    return (y,h) + (sims[y][h],)
##
##    (a, b) = random.choice(np.column_stack(found))
##    return (yeast_list[a], human_list[b], s[a][b])
##    list(zip(found[0], found[1])))
##    don't use this for numpy arrays. It runs slower.

def best_pair(pq, delta):
    try:
        pair_list = pq.pop(delta)
    except IndexError:
        raise StopIteration("no more pair values")
    return pair_list[1] 

def dijkstra(yeast_graph, human_graph, seed, sims, delta, num_seeds = 1):
    #global delta
    #delta above 0.01 takes a very long time to finish
    pq = SkipList() 
    pairs = set()
    yeast_nodes = set()
    human_nodes = set()

    seed_H, seed_Y = -1, -1

    while len(yeast_nodes) < len(yeast_graph):
        for _ in range(num_seeds):
            try: 
                seed_Y, seed_H = next(seed)
                while seed_Y in yeast_nodes or seed_H in human_nodes: #throw away bad seeds until you get a good one
                    seed_Y, seed_H = next(seed)
            except StopIteration: #no more seeds, end the loop
                return (yeast_nodes, human_nodes, pairs)

            yeast_nodes.add(seed_Y)
            human_nodes.add(seed_H)
            pairs.add((seed_Y, seed_H, sims[seed_Y][seed_H]))

        update_best_pair(pq, yeast_graph, human_graph, seed_Y, seed_H, pairs, sims, delta)
        while(True):
            try:
                curr_pair = best_pair(pq,delta)
                while curr_pair[0] in yeast_nodes or curr_pair[1] in human_nodes : #reject loop
                    curr_pair = best_pair(pq,delta)
                update_best_pair(pq, yeast_graph, human_graph, curr_pair[0], curr_pair[1], pairs, sims, delta)

                yeast_nodes.add(curr_pair[0])
                human_nodes.add(curr_pair[1])
                pairs.add(tuple(curr_pair) + (sims[curr_pair[0]][curr_pair[1]],))
            except StopIteration:
                break;
    return (yeast_nodes, human_nodes, pairs)




def induced_subgraph(graph1, graph2, aligned_pairs):
    result = []
    while aligned_pairs:
        p = aligned_pairs.pop()
        result.extend(
            [((p[0], q[0]), (p[1], q[1]))
                 for q in aligned_pairs
                    if graph1.has_edge(p[0],q[0])
                        and graph2.has_edge(p[1],q[1])])
    return result


def coverage(yeast, human, subgraph):
    y = len(subgraph) / (yeast.num_edges() / 2) * 100.0
    h = len(subgraph) / (human.num_edges() / 2) * 100.0
    return (y,h)


def write_result(file, pairs, graph1, graph2):
    with open(file, 'w+')as f:
        #f.write(str(len(d)) + ' ' + str(coverage(yeast_graph, human_graph,d)) + '\n')
        for x in pairs:
            f.write(str(graph1.nodes[x[0]]) + ' ' + str(graph2.nodes[x[1]]) + '\n')


def to_name(pairs, yd, hd):
    return [(yd[yeast_graph], hd[human_graph]) for (yeast_graph, human_graph, sims) in pairs]


#import datetime
def log_file_name(start = 'bionet_yeast_human', ext = '.txt'):
    dtime = datetime.datetime.now()
    return start + '_' +dtime.strftime("%Y-%m-%d_%H-%M-%S") + ext
