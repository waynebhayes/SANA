from builder import *
import structs
import copy
import numpy
from pathlib import Path
from collections import defaultdict
import time, datetime
from skip_list import *
import lzma
import sys
from measures import edgecoverage 
import uuid
import sys
import structs
sys.setrecursionlimit(10000)


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
    


def update_best_pair(pq, g1, g2, g1node, g2node, pairs, sims, delta = 0):
    paired_g1nodes = np.fromiter((pair[0] for pair in pairs), dtype=int)
    paired_g2nodes = np.fromiter((pair[1] for pair in pairs), dtype=int)
    g1node_neighbors = np.setdiff1d(g1.get_neighbors(g1node), paired_g1nodes)
    g2node_neighbors = np.setdiff1d(g2.get_neighbors(g2node), paired_g2nodes)

    if g1node_neighbors.size == 0 or g2node_neighbors.size == 0:
        return
    
    bp_list = sub_best_pair(g1node_neighbors, g2node_neighbors, sims, delta)
    for (val, new_pairs) in bp_list:
        if val >= 0:
            for pair in new_pairs:
                pq.add((val, pair))
    

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

def get_new_neighbor_pairs(g1, g2, node1, node2, g1alignednodes, g2alignednodes):
    result = set()
    
    for i in g1.get_neighbors(node1):
        for j in g2.get_neighbors(node2):
            if i not in g1alignednodes and j not in g2alignednodes:
                result.add((i,j))
    return result

#def get_candidate_pairs(g1,g2, node1, node2, g1alignednodes, g2alignednodes, g1candidatenodes, g2candidatenodes, edge_freq, simbound, sims):
#    
#    newcp = set()
#    existingcp = set()
#    
#    for i in g1.get_neighbors(node1):
#        for j in g2.get_neighbors(node2):
#            g1nodealigned = i in g1alignednodes
#            g2nodealigned = j in g2alignednodes
#            if not g1nodealigned and not g2nodealigned:
#                if sims[i][j] >= simbound:
#                    newcp.add((i,j))
#            elif not g1nodealigned:
#                for g2node in g1candidatenodes[i]:
#                    if (i, g2node) in edge_freq:
#                        existingcp.add((i,g2node))            
#            elif not g2nodealigned:
#                for g1node in g2candidatenodes[j]:
#                    if (g1node, j) in edge_freq:
#                        existingcp.add((g1node,j))            
#
#    return newcp, existingcp


def get_neighbor_candidate_pairs2(g1, g2, node1, node2, g1alignednodes, g2alignednodes, g1candidatenodes, g2candidatenodes, edge_freq):
    candidate_neighbors = set()
   
    for g1node in g1.get_neighbors(node1):
        if g1node not in g1alignednodes:
            for g2node in g1candidatenodes[g1node]:
                if (g1node, g2node) in edge_freq:
                    candidate_neighbors.add((g1node,g2node))            

    for g2node in g2.get_neighbors(node2):
        if g2node not in g2alignednodes:
            for g1node in g2candidatenodes[g2node]:
                if (g1node, g2node) in edge_freq:
                    candidate_neighbors.add((g1node,g2node))            

    return candidate_neighbors
    '''
    for i, j in neigbor_pairs:
        if (i, j) in edge_freq:
            candidate_neighbors.add(i,j)

    len(neighbor_pairs) < g1.get_neighbors(node1) + g2.get_neighbors(node2)
    '''

#def get_neighbor_candidate_pairs(g1, g2, node1, node2, g1alignednodes, g2alignednodes, edge_freq, sims):
#    new_neighbors = []
#    candidate_neighbors = []
#    for i in g1.get_neighbors(node1):
#        for j in g2.get_neighbors(node2):
#            if i not in g1alignednodes and j not in g2alignednodes:
#                if (i,j) in edge_freq:
#                    candidate_neighbors.append((i,j))            
#                else:
#                    if sims[i][j] >= simbound:
#                        new_neighbors.append((i,j))
#
#            #if i not in g1alignednodes and j not in g2alignednodes and (i,j) in edge_freq:
#            #    candidate_neighbors.append((i,j))            
#            #elif i not in g1alignednodes and j not in g2alignednodes:
#            #    if sims[i][j] > 0:
#            #        neighbors.append((i,j))
#    return new_neighbors, candidate_neighbors


def get_aligned_neighbor_pairs(g1,g2, node1,node2, aligned_pairs, trace = False):
    result = []
    
    for i in g1.get_neighbors(node1):
        for j in g2.get_neighbors(node2):
            if (i,j) in aligned_pairs:
                if trace:
                    print(i,j)
                result.append((i,j))
    return result

 
def num_edges_back_to_subgraph(graph, node, aligned_nodes, trace=False):
    edges = 0
  
    for neighbor_node in aligned_nodes:
        if graph.has_edge(node, neighbor_node):
            if trace:
                print("Nnode in graph : " + str(neighbor_node)) 
            edges += 1
    return edges
    '''

    for neighbor_node in graph.get_neighbors(node):
        if neighbor_node in aligned_nodes:
            edges += 1
    return edges

    '''

def num_edge_pairs_back_to_subgraph(g1, g2, g1node, g2node, aligned_pairs):
    edgepairs = 0
    #print("printing edges for M")
    for n1, n2 in aligned_pairs:
        if g1.has_edge(g1node, n1) and g2.has_edge(g2node, n2):
            #print(n1, n2)
            edgepairs += 1

    """
    for n1 in g1.get_neighbors(g1node):
        for n2 in g2.get_neighbors(g2node):
            if (n1, n2) in aligned_pairs:
                edgepairs += 1
    """
    return edgepairs

def update_skip_list(g1, g2, curralign, candidatePairs, debug):
    for g1node, g2node in candidatePairs:
        if g1node in curralign.g1alignednodes or g2node in curralign.g2alignednodes:
            if debug:
                print("updating edge_freq: ", (g1node, g2node), " already aligned")
            #delete this pair from edge_freq?
            #del curralign.edge_freq[(g1node, g2node)]
            continue
        n1 = num_edges_back_to_subgraph(g1, g1node, curralign.g1alignednodes)   
        n2 = num_edges_back_to_subgraph(g2, g2node, curralign.g2alignednodes)   
        M = num_edge_pairs_back_to_subgraph(g1, g2, g1node, g2node, curralign.aligned_pairs)            
        assert(M <= n1 and M <= n2), f"M={M}, n1={n1}, n2={n2}, nodes=({g1node},{g2node})"
        curralign.edge_freq[(g1node, g2node)] = [M, n1, n2]
        curralign.g1candidatenodes[g1node].add(g2node)
        curralign.g2candidatenodes[g2node].add(g1node)
        pair = (g1node, g2node)
        if debug:
            print(pair, " updated in edge_freq ", curralign.edge_freq[pair])
        #val = curralign.alpha*(curralign.edge_freq[pair][0]) + (1-curralign.alpha)*sims[pair[0]][pair[1]]
        val = curralign.edge_freq[pair][0]
        curralign.pq.add((val,pair), debug=debug)

class Alignment:
    def __init__(self, seed, m, ec_mode, ed, alpha, delta, seednum, outputdir,timestop):
        self.g1alignednodes = set()
        self.g2alignednodes = set()
        self.aligned_pairs = set()
        self.edge_freq = {}
        self.g1candidatenodes = defaultdict(set)
        self.g2candidatenodes = defaultdict(set)
        self.pq = SkipList()

        self.ec1 = ec_mode[0]
        self.ec2 = ec_mode[1]
        self.ed = ed
        self.E1 = m
        self.E2 = m
        self.EA = m
        self.g1seedstr = ""
        self.g2seedstr = ""
        self.k = 0
        self.seed = seed
        self.alpha = alpha
        self.delta = delta
        self.seednum = seednum
        self.currtime = 0 
        self.recdepth = 0
        self.timestop = timestop
        self.g1name = ""
        self.g1name = ""
        self.outputdir = outputdir

def writelog(curralign): 
    g1edges = induced_graph1(g1, curralign.aligned_pairs)
    g2edges = induced_graph2(g2,curralign.aligned_pairs)
    eaedges = induced_subgraph(g1,g2,curralign.aligned_pairs)
    print("ec1score: ", ec1score(len(g1edges)/2, len(eaedges)/2 ))
    print("ec2score: ", ec2score(len(g2edges)/2, len(eaedges)/2))
    print("s3score: ", s3score(len(g1edges)/2, len(g2edges)/2,len(eaedges)/2))


def printoutput(curralign):

    size = len(curralign.aligned_pairs)

    hours, rem = divmod(curralign.currtime, 3600)
    minutes, seconds = divmod(rem, 60)
    runtime = "{:0>2}:{:0>2}:{:05.2f}".format(int(hours),int(minutes),seconds)

    result = ("seednum: " + str(curralign.seednum) + " k:" + str(curralign.k) +  " size:" + str(size) + " E1:" + str(curralign.E1) + " E2:" + str(curralign.E2) + " EA:" + str(curralign.EA) + " time:" + str(runtime) + " seed: " + curralign.g1seedstr)
    
    if curralign.outputdir == "":
        output_dir =  "seed" + str(curralign.seednum)
    else: 
        output_dir = curralign.outputdir + "/seed" + str(curralign.seednum)
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    output_file = output_dir + "/" + curralign.logfile

    with open(output_file, "a") as f:
        f.write(result+"\n") 
        for n1, n2 in curralign.aligned_pairs:
            f.write(str(n1)+" "+str(n2)+"\n")


   
def rec_alignhelper(g1, g2, curralign, candidatePairs, aligncombs, debug):
    curralign.recdepth += 1
    printoutput(curralign)
    write_result(g1,g2,curralign)


    if curralign.timestop and curralign.currtime >= curralign.timestop:
        sys.exit()

    start1 = time.time()
   
    if len(candidatePairs) == 0 and len(curralign.pq) == 0:
        #print("No more candidatePairs, outputing alignment")
        return

    #print("CandidatePairs:", candidatePairs) 
    update_skip_list(g1, g2, curralign, candidatePairs, debug)

    bad_candidates = 0
    lastbad = None
    lastthrow = None

    seen = set()
    #curralign.pq.print_list()


    end1 = time.time()
    interval1 = end1 - start1
    curralign.currtime += interval1
    while(True): 
        try:
            if curralign.timestop and  curralign.currtime >= curralign.timestop:
                sys.exit()

            start2 = time.time()

            pair = best_pair(curralign.pq, curralign.delta)
            newcandidatePairs = set()
            g1node, g2node = pair
            bad_candidates += 1
            if debug:
                print("popping out", pair)

            if g1node in curralign.g1alignednodes or g2node in curralign.g2alignednodes or pair in seen:
                continue

            seen.add(pair)
            mval = curralign.edge_freq[pair][0]
            n1val = curralign.edge_freq[pair][1]
            n2val = curralign.edge_freq[pair][2]
            assert n1val >= mval and n2val >= mval, "mval is smaller than n1val and n2val"
            S=len(curralign.aligned_pairs)
            newed = (curralign.EA+mval)/(((S+1)*S)/2)
           
            if ((curralign.EA + mval)/(curralign.E1 + n1val)) < curralign.ec1 or ((curralign.EA + mval)/(curralign.E2 + n2val)) < curralign.ec2 or newed < curralign.ed: 
                

                candidatePairs.add(pair)
                continue
    

            print("Trying to add New Pair: " + str(pair) ,end=" ")

            #assert n1val == num_edges_back_to_subgraph(g1, pair[0], curralign.g1alignednodes), str(curralign.edge_freq[pair]) +  " "+ str(curralign.num_edges_back_to_subgraph(g1, pair[0], curralign.g1alignednodes))
            #assert n2val == num_edges_back_to_subgraph(g2, pair[1], curralign.g2alignednodes), str(curralign.edge_freq[pair]) + " " + str(num_edges_back_to_subgraph(g2, pair[1], curralign.g2alignednodes))
            #assert mval == num_edge_pairs_back_to_subgraph(g1, g2, pair[0], pair[1], curralign.aligned_pairs), str(curralign.edge_freq[pair])+ " " + str(mval) 
       

            s = len(curralign.aligned_pairs)
            print("S: ", len(curralign.aligned_pairs))
            print("recdepth: ", curralign.recdepth)
                

            #combination checking
            inserted = aligncombs.insertalignment(s+1, curralign.aligned_pairs.union({pair}))
            if not inserted:
                #print("Combination already exists, don't add that pair")
                continue
                


            #TODO 
            #make helper than updates all the data structures
            #make updates to alignment data structres based on pair
            curralign.aligned_pairs.add(pair)
            curralign.g1alignednodes.add(g1node)
            curralign.g2alignednodes.add(g2node)

            g1candtemp = curralign.g1candidatenodes[g1node]
            g2candtemp = curralign.g2candidatenodes[g2node]
            del curralign.g1candidatenodes[g1node]
            del curralign.g2candidatenodes[g2node]

            newcandidatePairs = get_new_neighbor_pairs(g1,g2,pair[0],pair[1], curralign.g1alignednodes, curralign.g2alignednodes)
             
            exisiting_neighbor_candidatePairs = get_neighbor_candidate_pairs2(g1, g2, pair[0], pair[1], curralign.g1alignednodes, curralign.g2alignednodes, curralign.g1candidatenodes, curralign.g2candidatenodes, curralign.edge_freq) 




            #TODO 
            #make into helper function
            for p in exisiting_neighbor_candidatePairs:
                val = curralign.edge_freq[p][0]
                curralign.pq.remove_by_name(val, p)

            #TODO
            #make into helper function
            curralign.E1 += n1val
            curralign.E2 += n2val
            curralign.EA += mval

            if debug:
                print("E1: " + str(curralign.E1),end=" ")
                print("E2: " + str(curralign.E2),end=" ")
                print("EA: " + str(curralign.EA),end=" ")
    

            end2 = time.time()
            interval2 = end2 - start2
            curralign.currtime += interval2            

            if debug:
                print("Adding", curralign.recdepth, pair)
                #print("existing ncp before call:", exisiting_neighbor_candidatePairs)
            #recursive call on newcurralign which has added the current pair and updated data structures
            rec_alignhelper(g1,g2, curralign, candidatePairs.union(newcandidatePairs), aligncombs, debug)
        
            start3 = time.time()
            

            if debug:
                print("Reverting", curralign.recdepth-1, pair)

            # remove the new neighbor cp 
            for node1, node2 in newcandidatePairs:
                #val = curralign.alpha*(curralign.edge_freq[(node1, node2)][0]) + (1-curralign.alpha)*sims[node1][node2]
                val = curralign.edge_freq[(node1, node2)][0]
                curralign.pq.remove_by_name(val, (node1, node2))
                del curralign.edge_freq[(node1,node2)]

            #revert all data structures
            curralign.recdepth -= 1
            curralign.aligned_pairs.remove(pair) 
            curralign.g1alignednodes.remove(g1node) 
            curralign.g2alignednodes.remove(g2node) 
            curralign.g1candidatenodes[g1node] = g1candtemp
            curralign.g2candidatenodes[g2node] = g2candtemp

            
            #remove and reinsert the neighbor cp that already exists
            for node1, node2 in exisiting_neighbor_candidatePairs.union(candidatePairs):
                n1 = num_edges_back_to_subgraph(g1, node1, curralign.g1alignednodes)   
                n2 = num_edges_back_to_subgraph(g2, node2, curralign.g2alignednodes)   
                M = num_edge_pairs_back_to_subgraph(g1, g2, node1, node2, curralign.aligned_pairs)            
                assert(M <= n1 and M <= n2), f"M={M}, n1={n1}, n2={n2}, nodes=({g1node},{g2node})"
                curralign.edge_freq[(node1, node2)] = [M, n1, n2]
                #val = curralign.alpha*(curralign.edge_freq[(node1,node2)][0]) + (1-curralign.alpha)*sims[node1][node2]
                val = curralign.edge_freq[(node1, node2)][0]
                curralign.pq.add((val, (node1,node2))) 
                #print("Add", (node1, node2), "to skiplist")


            curralign.E1 -= n1val
            curralign.E2 -= n2val
            curralign.EA -= mval

            candidatePairs.add(pair)
                    
            end3 = time.time()
            interval3 = end3 - start3
            curralign.currtime += interval3

        except StopIteration:
            if debug:
                print("No valid candidate pairs in skiplist, returning..")
            return


def rec_align(g1, g2, seed, ec_mode, ed, m, delta, alpha, seednum, outputdir, timestop=None, debug=False):
        
    curralign = Alignment(seed, m, ec_mode, ed, alpha, delta, seednum, outputdir, timestop)
    curralign.logfile = g1.name + "_" + g2.name + "_" + str(seednum) + ".log" 

    candidatePairs = set()
    if debug:
        print("aligning inital seeds*************************************************************************")
        print("graph 1:")
        for i in range(len(g1)):
            print(i,":",g1.get_neighbors(i))
        print("graph 2:")
        for i in range(len(g2)):
            print(i,":",g2.get_neighbors(i))

    for seed1, seed2 in seed:
        if debug:
            print((seed1,seed2))
        curralign.g1seedstr += str(g1.nodes[seed1]) + " " 
        curralign.g2seedstr += str(g2.nodes[seed2]) + " " 
        curralign.aligned_pairs.add((seed1, seed2))
        curralign.g1alignednodes.add(seed1)
        curralign.g2alignednodes.add(seed2)
        candidatePairs.update(get_new_neighbor_pairs(g1,g2,seed1,seed2,curralign.g1alignednodes, curralign.g2alignednodes)) 

    curralign.g1seedstr += curralign.g2seedstr
    curralign.k = len(curralign.aligned_pairs)

    print('initial alignment:',curralign.aligned_pairs)

    if(debug):
        print("ec1: " + str(curralign.ec1))
        print("ec2: " + str(curralign.ec2))

    aligncombs = structs.alignCombinations()
    rec_alignhelper(g1, g2, curralign, candidatePairs, aligncombs,debug)

def output(k, E1, E2, EA, seed, runtime, seednum, size):
    print("seednum: " + str(seednum) + " k:" + str(k) +  " size:" + str(size) + " E1:" + str(E1) + " E2:" + str(E2) + " EA:" + str(EA) + " time:" + str(runtime) + " seed: " + str(seed))



def induced_subgraph(graph1, graph2, aligned_pairs):
    result = []
    for p in aligned_pairs:
        result.extend(
            [((p[0], q[0]), (p[1], q[1]))
                 for q in aligned_pairs
                    if graph1.has_edge(p[0],q[0])
                        and graph2.has_edge(p[1],q[1])])
    return result

def induced_graph1(graph1, aligned_pairs):
    result = []
    for p in aligned_pairs:
        result.extend(
            [((p[0], q[0]), (p[1], q[1]))
                 for q in aligned_pairs
                    if graph1.has_edge(p[0],q[0])])
    return result

def induced_graph2(graph2, aligned_pairs):
    result = []
    for p in aligned_pairs:
        result.extend(
            [((p[0], q[0]), (p[1], q[1]))
                 for q in aligned_pairs
                    if graph2.has_edge(p[1],q[1])])
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

#def s3score(g1, g2, pairs, subgraph):
#    aligned_edges = len(subgraph)
#    u2in = unaligned_edges_g2_in(g1, g2, pairs, subgraph)
#    denom = aligned_edges + (g1.num_edges() / 2) + len(u2in) 
#    return aligned_edges/denom


def ec1score(E1, EA):
    return EA/E1

def ec2score(E2, EA):
    return EA/E2

def s3score(E1,E2, EA):
    return EA/(E1+E1-EA)    


def write_result(g1,g2, curralign):
    uuidstr = str(uuid.uuid4())
    uid = uuidstr[:13]
    fname = g1.name + "--" + g2.name + "--" + str(curralign.delta) + "--" + str(curralign.k) + "--"  + uid +  ".dijkstra"

    if curralign.outputdir == "":
        output_dir =  "seed" + str(curralign.seednum)
    else: 
        output_dir = curralign.outputdir + "/seed" + str(curralign.seednum)
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    output_file = output_dir + "/" + fname
    with open(output_file, 'w+')as f:
        for x in curralign.aligned_pairs:
            print(str(g1.nodes[x[0]]) + ' ' + str(g2.nodes[x[1]]))
            f.write(str(g1.nodes[x[0]]) + ' ' + str(g2.nodes[x[1]]) + '\n')

def to_name(pairs, yd, hd):
    return [(yd[yeast_graph], hd[human_graph]) for (yeast_graph, human_graph, sims) in pairs]


#import datetime
def log_file_name(start = 'bionet_yeast_human', ext = '.txt'):
    dtime = datetime.datetime.now()
    return start + '_' +dtime.strftime("%Y-%m-%d_%H-%M-%S") + ext
