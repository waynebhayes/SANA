rom builder import *
import numpy
##import random
from collections import defaultdict
import datetime
from skip_list import *
import lzma
import seeding

def get_aligned_seed(s, graph1, graph2):
    for pair in s:
        #yield [int(pair[0]),int(pair[1])]
        yield [graph1.indexes[pair[0]], graph2.indexes[pair[1]]]


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

    #low = 1
    #low = max(s.max() - delta, 0.0)
    low = 0
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


def fit_in_curr_align(g1, g2, node1, node2, pairs):
    neighbor1 = g1.get_neighbors(node1)
    #neighbor1 = g1.get_neighbors(g1.indexes[node1])
    neighbor2 = g2.get_neighbors(node2)
    #neighbor2 = g2.get_neighbors(g2.indexes[node2])
    flipped = False
    if len(neighbor1) > len(neighbor2):
        g1, g2 = g2, g1
        node1, node2 = node2, node1
        neighbor1, neighbor2 = neighbor2, neighbor1
        flipped = True 
    curr_alignment = {pair[1]:pair[0] for pair in pairs} if flipped else {pair[0]:pair[1] for pair in pairs}

    #n2 = []
    #for node in neighbor2:
    #    n2.append(g2.nodes[node])
    
    for node in neighbor1:
        if node in curr_alignment and curr_alignment[node] not in neighbor2:
        #if g1.nodes[node] in curr_alignment and curr_alignment[node] not in n2:
            return False
    return True


def get_neighbor_pairs(g1, g2, node1, node2, sims, delta = 0):
    result = []
    for i in g1.get_neighbors(node1):
    #for i in g1.get_neighbors(g1.indexes[node1]):
        for j in g2.get_neighbors(node2):
        #for j in g2.get_neighbors(g2.indexes[node2]):
            if sims[i][j] != 0:
                result.append((i,j))
    return result


def strict_align(g1, g2, seed, sims, delta = 0):
    used_node1 = set()
    used_node2 = set()
    stack = []
    pairs = set()
    # initialize
    #print(g1.nodes)
    for seed1, seed2 in seed:
        #pairs.add((seed1, seed2, sims[seed1][seed2]))
        #print(g1.nodes[seed1])
        #print(seed1)
        pairs.add((seed1, seed2, sims[seed1][seed2]))
        used_node1.add(seed1)
        used_node2.add(seed2)
        stack += get_neighbor_pairs(g1,g2,seed1,seed2,sims) 

    # while we still have still nodes to expand
    while stack:
        node1, node2 = stack.pop()
        if node1 not in used_node1 and node2 not in used_node2:
            used_node1.add(node1)
            used_node2.add(node2)
            if fit_in_curr_align(g1,g2,node1,node2,pairs):
                pairs.add((node1,node2,sims[node1][node2]))
                #pairs.add((seed1, seed2, sims[g1.indexes[seed1]][g2.indexes[seed2]]))
                stack += get_neighbor_pairs(g1,g2,node1,node2,sims)
    return (used_node1, used_node2, pairs)


def stop_align2(g1, g2, seed, sims, delta = 0):
    g1alignednodes = set()
    g2alignednodes = set()
    aligned_pairs = set()
    pq = SkipList()
    stack = []
    
    for seed1, seed2 in seed:
        aligned_pairs.add((seed1, seed2, sims[seed1][seed2]))
        g1alignednodes.add(seed1)
        g2alignednodes.add(seed2)
        stack += get_neighbor_pairs(g1,g2,seed1,seed2,sims) 
        update_best_pair(pq, g1, g2, seed1, seed2, aligned_pairs, sims, delta)


    while len(g1alignednodes) < len(g1):
        try:
            curr_pair = best_pair(pq, delta)
            g1node = curr_pair[0]
            g2node = curr_pair[1]
            if g1node in g1alignednodes or g2node in g2alignednodes:
                continue
            if fit_in_curr_align(g1, g2, g1node, g2node, aligned_pairs):
                update_best_pair(pq, g1, g2, g1node, g2node, aligned_pairs, sims, delta)
                aligned_pairs.add((g1node, g2node, sims[g1node][g2node]))
                g1alignednodes.add(g1node)
                g2alignednodes.add(g2node)
        except(StopIteration): 
            break

    return (g1alignednodes, g2alignednodes, aligned_pairs)




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
                
                pairs.add(tuple(curr_pair) + (sims[curr_pair[0]][curr_pair[1]],))
                yeast_nodes.add(curr_pair[0])
                human_nodes.add(curr_pair[1])
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

def unaligned_edges_g2_in(graph1, graph2, aligned_pairs, subgraph):
    uedges = []
    while aligned_pairs:
        p = aligned_pairs.pop()
        uedges.extend( [ (p[1], q[1]) for q in aligned_pairs if graph2.has_edge(p[1], q[1]) and ((p[0], q[0]), (p[1], q[1])) not in subgraph ]   )
    return uedges


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
