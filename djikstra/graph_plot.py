from builder import *
import sys
##import random
from collections import defaultdict
import datetime
#from myqueue import *
from skip_list import *

def get_seed(file):
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
    curr_value = 1.0
    with open(file) as f3:
        for line in f3:

            row = line.strip().split()
            row[0], row[1], row[2] = int(row[0]), int(row[1]), 1- float(row[2])
            if row[2] < curr_value:
##                tree_trace.append(('s',len(tied_seeds)))
                random.shuffle(tied_seeds)
##                    print("this node has lower similarity\n")
                for seed in tied_seeds:
                    yield seed[0:2]
                del tied_seeds
                tied_seeds = [row[0:2]]
                curr_value = row[2]
##                    print("reset tied seeds" + str(len(tied_seeds)))
            else:
                tied_seeds.append(row[0:2])
        random.shuffle(tied_seeds)
##        tree_trace.append(('s',len(tied_seeds)))
        for seed in tied_seeds:
            yield seed[0:2]

def update_best_pair(pq, yeast_graph, human_graph, yeast_node, human_node, pairs, sims, delta = 0):
##    nonlocal pq
##    print("looking for neighbors of ", (yeast_node, human_node))
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
    
    # print("\nYeast  ", yeast_neighbors, "\n\n Human", human_neighbors)
    bp_list = sub_best_pair(yeast_neighbors, human_neighbors, sims, delta)
##    print(bp_list)
    for (val, new_pairs) in bp_list:
        if val >= 0:
            for pair in new_pairs:
##                print((val, pair))
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
    #print("\nYeast  ", yeast_neighbors, "\n\n Human", human_neighbors)
    if yeast_neighbors[-1] >= len(sims):
        print("Uh oh\n\\n\n")
    
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

def best_pair(pq):
    try:
        pair_list = pq.pop(delta)
    except IndexError:
        raise StopIteration("no more pair values")
    #print(type(pair_list))
    val=pair_list[0]
    pair_arr=pair_list[1]
    #print(pair_list)
    return pair_arr 

##    np.random.shuffle(pair_arr)
##    pair = pair_arr[0] #pick the first pair from the list
##    rest_of_pairs = np.delete(pair_arr, 0, axis = 0) #axis=0 to delete the pair, not a value from each pair
##    if rest_of_pairs.size > 0:
##        pq.requeue((val, rest_of_pairs))
##    return pair 
    
    """
    try:
        pair_list = pq.pop_equals()
##        print(pair_list)
    except IndexError:
        raise StopIteration("no more pair values")
    if len(pair_list) < 1:
        raise EmptyList("list is empty")
    random.shuffle(pair_list)
    val, pair_select = pair_list[0] #pick the first pair list e.g (1, [[1, 2]])
    rest_of_pairs = pair_list[1:]
    if len(rest_of_pairs) > 0:
        pq.enqueue(rest_of_pairs)
    np.random.shuffle(pair_select)
    pair = pair_select[0] #pick the first pair from the list
    rest_of_pairs = np.delete(pair_select, 0, axis = 0) #axis=0 to delete the pair, not a value from each pair
    if rest_of_pairs.size > 0:
        pq.insert((val, rest_of_pairs))
    return pair
"""    
def dijkstra(yeast_graph, human_graph, seed, sims, num_seeds = 1):
    global delta
    #delta above 0.01 takes a very long time to finish
    # 0.05 ~ 10 min
    pq = SkipList() #my_queue(delta)
    pairs = set()
    yeast_nodes = set()
    human_nodes = set()

    seed_H, seed_Y = -1, -1
    print('Len of sims ', len(sims))
    while len(yeast_nodes) < len(yeast_graph):
        for _ in range(num_seeds):
            try: #get seeds
                seed_H, seed_Y = next(seed)
                while seed_Y in yeast_nodes or seed_H in human_nodes: #throw away bad seeds until you get a good one
                    seed_H, seed_Y = next(seed)
            except StopIteration: #no more seeds, end the loop
                return (yeast_nodes, human_nodes, pairs)

            yeast_nodes.add(seed_Y)
            human_nodes.add(seed_H)
            pairs.add((seed_Y, seed_H, sims[seed_Y][seed_H]))

        update_best_pair(pq, yeast_graph, human_graph, seed_Y, seed_H, pairs, sims, delta)
        while(True):
            try:
                curr_pair = best_pair(pq)
                while curr_pair[0] in yeast_nodes or curr_pair[1] in human_nodes : #reject loop
                    curr_pair = best_pair(pq)
                update_best_pair(pq, yeast_graph, human_graph, curr_pair[0], curr_pair[1], pairs, sims, delta)

                yeast_nodes.add(curr_pair[0])
                human_nodes.add(curr_pair[1])
                pairs.add(tuple(curr_pair) + (sims[curr_pair[0]][curr_pair[1]],))
            except StopIteration:
                break;
    return (yeast_nodes, human_nodes, pairs)


def induced_subgraph(graph1, graph2, aligned_pairs):
    # 
    result = []
    while not not aligned_pairs:
        p = aligned_pairs.pop()
        for q in aligned_pairs:
            if graph1.has_edge(p[0],q[0]) and graph2.has_edge(p[1],q[1]):
                result.append(((p[0], q[0]), (p[1], q[1])))
##        result.extend(
##            [((p[0], q[0]), (p[1], q[1]))
##                 for q in aligned_pairs
##                    if graph1.has_edge(p[0],q[0])
##                        and graph2.has_edge(p[1],q[1])])
    return result


def coverage(yeast, human, subgraph):
    y = len(subgraph) / (yeast.num_edges() / 2) * 100.0
    h = len(subgraph) / (human.num_edges() / 2) * 100.0
    return (y,h)


def write_result(file, pairs):
    with open(file, 'w+')as f:
        f.write(str(len(d)) + ' ' + str(coverage(yeast_graph, human_graph,d)) + '\n')
        for x in pairs:
            f.write(str(x) + '\n')


def to_name(pairs, yd, hd):
    return [(yd[yeast_graph], hd[human_graph]) for (yeast_graph, human_graph, sims) in pairs]

#import datetime
def log_file_name(start = 'bionet_yeast_human', ext = '.txt'):
    dtime = datetime.datetime.now()
    return start + '_' +dtime.strftime("%Y-%m-%d_%H-%M-%S") + ext

if __name__ == "__main__":
    if len(sys.argv) < 7:
        raise ValueError("Did not receive enough arguments")
    test = sys.argv[1]
    yeast_graph = build_graph(sys.argv[1]) ## yeasti.txt
    human_graph = build_graph(sys.argv[3]) ## humani.txt
    sims = get_sim(sys.argv[5], len(yeast_graph), len(human_graph)) ## simsi.txt
    global delta
    delta= float(sys.argv[6])
    yeast_dict = read_dict(sys.argv[2]) ## yeastd.txt
    human_dict = read_dict(sys.argv[4]) ## humand.txt

    """
    don't use rsorted_simsi.txt ! Since the similarity is reversed,
    we should also reverse the file and use sorted_simsi.txt instead!
    """
##    global tree_trace
##    tree_trace = list()
#    for _ in range(1):
#        a,b,c = dijkstra(yeast_graph, human_graph, get_seed("sorted_simsi.txt"), sims)
#        d = induced_subgraph(yeast_graph, human_graph, list(c))
#
#        print(coverage(yeast_graph, human_graph,d))
#        write_result(log_file_name('result1'),c)
#        write_result(log_file_name('result2'),to_name(c, yeast_dict, human_dict))

#with time record
    import matplotlib.pyplot as plt
    import time
#    bias=[]
#    #fullrand=[]
#    k=20
#    for i in range(k):
#        starttime=time.time()
#        a,b,c = dijkstra(yeast_graph, human_graph, get_seed("sorted_simsi.txt"), sims)
#        d = induced_subgraph(yeast_graph, human_graph, list(c))
#        cov=coverage(yeast_graph, human_graph,d)[0]
#        print(cov)
#        bias.append(cov)
#    plt.boxplot(bias,fullrand)
#    plt.title("ec different algorithm. k="+str(k))
#    plt.xlabel("left:bias right:full_rand")
#    plt.ylabel("edge coverage")
#    plt.figure()

    result=[]
    time_record=[]        
    
    ss=10
    for i in range(0,10):
        result.append([])
        time_record.append([])
      ##  delta=(i**2)*0.0025
        for j in range(ss):
            start_time = time.clock()
            
            a,b,c = dijkstra(yeast_graph, human_graph, get_seed(sys.argv[5]), sims)
            d = induced_subgraph(yeast_graph, human_graph, list(c))
            cov=coverage(yeast_graph, human_graph,d)[0]
            print(cov)
            result[i].append(cov)
            
            time_used = time.clock() - start_time
            time_record[i].append(time_used)
            
           ### file_name='delta='+str(delta)+'_time='+str(int(time_used))+"_"
            ##print(str(len(d)) + ' ' + str(coverage(yeast_graph, human_graph,d)) + '\n')
            for x in c:
                print(str(x), '\n')

           ### write_result(log_file_name(file_name),c)
        time_record[i]=np.mean(time_record[i])
            
    plt.boxplot(result)
    plt.title("different delta for ec (rand in range), sample size="+str(ss))
    plt.xlabel("delta=(i-1)^2 * 0.0025")
    plt.ylabel("edge_coverage")
    plt.figure()
    
    plt.plot(time_record)
    plt.title("different delta for ec (rand in range), sample size="+str(ss))
    plt.xlabel("delta=(i-1)^2 * 0.0025")
    plt.ylabel("time_used")
    plt.figure()
        
#    start_time = time.clock()
#    print("start ")
#    print(start_time)
#    result=[]
#    for i in range(10):
#        delta=(10*2)*0.0025
#        a,b,c = dijkstra(yeast_graph, human_graph, get_seed("sorted_simsi.txt"), sims)
#        d = induced_subgraph(yeast_graph, human_graph, list(c))
#        cov=coverage(yeast_graph, human_graph,d)[0]
#        result.append(cov)
#        print(cov)
#    plt.boxplot(result)
#    plt.title("different delta for ec (rand in range), sample size="+str(ss))
#    plt.xlabel("delta=(i-1)^2 * 0.0025")
#    plt.ylabel("edge coverage")
#    plt.figure()
#    print("end ", time.clock() - start_time)









