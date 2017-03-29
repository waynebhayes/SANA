from graph import Graph
import pickle
import numpy as np
"""
Don't try to replace the pickle function with the numpy save/load.
It's 1) a waste of time and 2) less space efficient
"""
"""
build_graph() takes a file in the following format:
Node 1 Node 2
and returns a graph with every edge Node 1 <-> Node 2
"""
def build_graph(file):
    g = Graph()
    with open(file) as f:
        for line in f:
            node0, node1 = [int(n) for n in line.strip().split(" ")]
            g.add_edge(node0, node1)
    return g
"""
get_sim() provides a wrapper to speed up the construction of the sims matrix.
It stores the matrix in a pickle to minimize calls to build_sim. Future calls
to this function will just load the pickle, which is faster for large datasets
than calling build_sims().
"""
def get_sim(file, yeast_size, human_size, pickle_name = ".sim.pickle"):
    """
    build_sim takes a file and builds a [yeast_size X human_size]
    array of similarity values. This function takes a long time to run
    for large datasets so it should be avoided when possible.
    runtime: O(|Y| * |H|)
    """
    def build_sim(file, yeast_size, human_size):
##        similarity = [x[:] for x in [[0.0] * human_size] * yeast_size]
        similarity = np.zeros((yeast_size, human_size))
        with open(file) as f:
            for line in f:
                node_h, node_y, sim_val = line.strip().split(" ") # 8 seconds
                node_y, node_h, sim_val = int(node_y), int(node_h), 1 - float(sim_val) #17 seconds
                similarity[node_y][node_h] = sim_val

        return similarity
    
    if ".sim.pickle" == pickle_name:
        pickle_name = file + pickle_name
    try:
        with open(pickle_name,'rb') as f:
            return pickle.load(f)
    except FileNotFoundError as e:
        sims = build_sim(file, yeast_size, human_size)
        with open(pickle_name,'wb') as f:
            pickle.dump(sims,f)
        return sims
"""
functions below are not working, stubs for future releases
"""
def read_dict(file):
    """
    file = open('yeastd.txt','r')
    line = file.readline()
    d = dict()
    while line:
        row = line.strip().split(" ")
        d[int(row[0])] = row[1]
        line = file.readline()
    file.close()
    """
    d = dict()
    with open(file, 'r') as f:
        for line in f:
            row = line.strip().split(" ")
            d[int(row[0])] = row[1]
    return d

##def build_to_number(file):
##    to_number = dict()
##    with open(file) as f:
##        for line in f.readlines():
##            pair = line.strip().split(" ")
##            to_number[pair[1]] = pair[0]
##    return to_number
##
##def build_to_name(file):
##    to_name = dict()
##    with open(file) as f:
##        for line in f.readlines():
##            pair = line.strip().split(" ")
##            to_name[pair[0]] = pair[1]
##    return to_name
