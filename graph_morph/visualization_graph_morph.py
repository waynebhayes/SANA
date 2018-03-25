

#Work In Progress

'''

currently the expected input is:
graph_morph -t transition_number(integer) edgelist1(file path) edgelist2_file(file path) output_file(file path)
graph_morph -c weight1(float). . . weightn(float) edgelist1(file path) . . .  edgelist2(file path)
'''



#Note that graphs are all assumed to be undirected
import sys
import math
import networkx as nx
import matplotlib.pyplot as plt
import numpy as np
import random
import matplotlib.animation as animation
import matplotlib as mpl
import scipy
from collections import defaultdict


INPUT_MSG = "currently the expected input is:\nfor transform: graph_morph -t transition_number(integer) edgelist1(file path) edgelist2_file(file path) output_file(file path)\nfor linear combination: graph_morph -c weight1(float). . . weightn(float) edgelist1(file path) . . .  edgelist2(file path)\nDoing the transfrom -t command requires ImageMagick and having .gif format support with it"


MULTI_COLORS = ['green', 'red', 'purple', 'orange', 'cyan', 'blue', 'yellow', 'brown']
NODES = 40
TOTAL = 20
PATHS = []
OUTFILE = "out"
WEIGHTS = []
PATH1 = "ER_2000_0.007_12"
PATH2 = "ER_2000_0.007_11"
MULTI = True
FRAME_DELAY = 200 #delay between frames
COLOR = mpl.colors.ListedColormap(['white', 'cyan', 'red', 'black']) #discrete color map
NORM = mpl.colors.BoundaryNorm([0,1,2,3,4], COLOR.N)    #maps colors to intervals (presumably inclusive) of numbered values
REPEAT_DELAY = 2500
START = 1
END =  2
EMPTY = 0
BOTH = 4

TEST_WITH_RANDOM = False

class InputError(Exception):
    pass

def makeRandGraph(n, color): #creates random graph with given color and node size
    
    G = nx.Graph()
    G.add_nodes_from(i for i in range(n))
    edgeIter(G, n, lambda x, y: color * (random.randint(0, 100) % 7 == 0))
    
    
    return G

def edgeIter(G, n, pred): #iterates through all the (n choose 2) possible edges for a graph, assigns it as an edge based on function pred
    i = 0
	
    while i < n:
        j = 0
        while j < i + 1:
            color = pred(i, j)
            if(color != EMPTY):
                G.add_edge(i, j, weight=color)
            j  += 1
        
        i += 1

def prob_val(alpha, total ): #value from probability model, based on current state alpha and number of states total
    return random.random() <= 1/(total - (alpha - 1))

'''
For linear combination:
Removing an edge from G_0 depends on weight, the current iteration number and total iteration number.
p(0,alpha) = (1 - weight_0)/(total - (alpha-1)(1-weight_0)).
Adding an edge from G_i depends on w_i and the current iteration number and total iteration number.
p(i,alpha) = (1 - weight_i) / (total - (alpha-1)(1-weight_i)).
'''
def prob_multi(weight, alpha, total):
    
    return random.random() <= (1 - weight)/(total - (alpha - 1)*(1 - weight))

def updateColor( g, curg, alpha, endg, total): #Removed Start colors of current graph and adds End colors, based on a probability model. Takes current graph to modify g, remaining edges in current graph g, current state alpha, remaining edges in ending graph endg, number of states total 
    edges_to_delete = set()
    for g_edge in curg:
        
        if prob_val(alpha, total):
            g.remove_edge(*g_edge)
            edges_to_delete.add(g_edge)
    curg.difference_update(edges_to_delete)

    edges_to_delete.clear()

    for final_edge in endg:
        if prob_val(alpha, total):
            g.add_edge(*final_edge, weight=END)
            edges_to_delete.add(final_edge)
    endg.difference_update(edges_to_delete)

          
    
            
            
def getEdgesCommon(sets):
    freq = defaultdict(int)
    edge_color = dict()
    colors = range(1, len(sets) + 1)
    for s, count in zip(sets, range(len(sets))):
        for edge in s:
            freq[edge] += WEIGHTS[count]
            
    combo_graph = nx.Graph()
    for edge in freq:
        if random.random() < freq[edge]: 
            combo_graph.add_edge(*edge, weight=max(*colors, key=lambda x: WEIGHTS[x - 1] if edge in sets[x-1] else -1))
            
            
    return combo_graph
    
    
    
                
def setCommonColors(g, endg): #Before any transitions are applied and edges are added/removed, all common edges shared by g and endg are marked with color BOTH in graph g
    for edge in g.edges():
        if edge in endg:
            g[edge[0]][edge[1]]['weight'] = BOTH
        
     
def alpha_iter(total): #iterator that yields every transition state from start (0) to the amount of stats (total)
    i = 0
    while i <= total + 1:
        yield i
        i += 1


def file_to_graph(filename, color): #returns networkx file with desird color from an edgelist file
    try:
        returned_graph = nx.read_edgelist(filename, nodetype=int)
    except:
        print("Error: could not read and convert the edgelist file" + filename + " to a graph.\n Ensure all characters in file are integers")
        raise

    for edge in returned_graph.edges():
        
        returned_graph[edge[0]][edge[1]]['weight'] = color

    return returned_graph
    
   
def parse_cmd(args):
    
    global MULTI
    global OUTFILE
    global NODES
    MULTI = False
    if len(args) < 4:
        print(INPUT_MSG)
        raise InputError  
    elif args[1] in ("-t", "-tw"):
        MULTI = False
    elif args[1] in ("-c", "-cw"):
        MULTI = True
    else:
        print(INPUT_MSG)
        raise InputError
    if not MULTI:
        global TOTAL
        TOTAL = int(args[2])
        global PATH1
        global PATH2
        PATH1, PATH2 = args[3], args[4]
        OUTFILE = args[5];
    else:
        fileAmt = len(args)//2 - 1
        if fileAmt > len(MULTI_COLORS):
            print("Error: currently only up to " + str(len(MULTI_COLORS)) + " files allowed")
            raise InputError
        print(fileAmt)
        global PATHS
        global WEIGHTS
        PATHS = []
        WEIGHTS = []
        for filename, proportion in zip(args[fileAmt + 2:2 * fileAmt + 2], args[2:fileAmt + 2]):
            PATHS.append(filename)
            WEIGHTS.append(float(proportion))
        
        
if __name__ == '__main__':

    
    parse_cmd(sys.argv)
    write_lambda =  lambda graph, write_file: nx.write_edgelist(graph, write_file)
    if 'w' in sys.argv[1]:
        write_data = write_lambda
    else:
        write_data = lambda x,y: x
        
    total = TOTAL 
    if MULTI: #Setting up data structures and control flow for if transformation is a linear combination of 3+ graphs
        gcolors = [i for i in range(len(PATHS) + 3)]
        COLOR =mpl.colors.ListedColormap(['white', *MULTI_COLORS[:len(PATHS)], 'black'])
        print("colors of graphs in the order they were passed are " + str(MULTI_COLORS[:len(PATHS)]) + ", black cooresponds to some of the shared edges\nCurrently only 8 different graphs allowed")
        NORM = mpl.colors.BoundaryNorm(gcolors, COLOR.N)
       
        START = 1
        BOTH = len(PATHS) + 3
        graphs = []
        different_edges = []
        sharedg = set()
        count = 1
        for filename in PATHS:
            graphs.append( file_to_graph(filename, count) if not TEST_WITH_RANDOM else makeRandGraph(NODES, count))
            count += 1
            
        for g_i in graphs:
            s_i = set(g_i.edges())
            different_edges.append(s_i)
               
       
        sharedg = getEdgesCommon(different_edges)
        fig = plt.figure(figsize=(10,10))
        sparse = nx.to_scipy_sparse_matrix(sharedg).tocoo()
        write_file = ""
        for filename, weight in zip(PATHS, WEIGHTS):
            write_file += filename + "_" + str(weight) + "," 
        write_data(sharedg, write_file)
        size = 1/math.sqrt(len(sparse.data) // 8)
        plt.scatter(sparse.row, sparse.col, c=sparse.data,  cmap=COLOR, norm=NORM, s= size)
        plt.show()
        plt.close()
        
      
        #COLOR =mpl.colors.ListedColormap(['white', *MULTI_COLORS, 'black']) 
        #NORM = mpl.colors.BoundaryNorm([i for i in range(len(graphs) + 2)], COLOR.N)
        
        
        
    else:#Setting up data structures and control flow for if its one graph transforming directly to another
        #setting up graphs. g1 and g2 are start and end graphs respectively. g0 (a deep copy of g1) will be used as current graph state for animation/transformation
        g1 = file_to_graph(PATH1, START) if not TEST_WITH_RANDOM else makeRandGraph(NODES , START)
      
        g0 = g1.copy()
        g2 = file_to_graph(PATH2, END) if not TEST_WITH_RANDOM else makeRandGraph(NODES, END)

        
        g2_set = set(g2.edges())
        #takes common edges in g1 and g2, and colors them with BOTH
        setCommonColors(g1, g2_set)
        

        #sets up edges from g1 and g2, current_edges has edges only in g1 and not g2, ending_edges has edges only in g2 and not g1
        current_edges = set(g1.edges())
        ending_edges = g2_set
        current_edges.difference_update(ending_edges)
        ending_edges.difference_update(g1.edges())

        #end of setup fo transformaton graphs g1 and g2
        
        n = NODES

        size = 1/math.sqrt(n // 8)
        fig = plt.figure(figsize=(10,10))

        #end of data structure/object setup


        #animation functions defined in __main__ scope so that only frame number needs to be passed as an argument
        #animation functions passed to pyplot FuncAnimation. Possibily can still be optimized more. 

        
        #animation function for basic graph transformation
        def animate_singular_color(frame): 
        
            updateColor(g1,  current_edges, frame,  ending_edges, total)
            
            plt.clf()
            write_data(g1, PATH1 + "_to_" + PATH2 + "_" + str(frame))
            
            sparse = nx.to_scipy_sparse_matrix(g1).tocoo()
        
            #draws the actual picture using pyplot that will be in animation
            plt.scatter(sparse.row, sparse.col, c=sparse.data,  cmap=COLOR, norm=NORM, s= size)

        update_m = animate_multi if MULTI else animate_singular_color
    
        g_ani = animation.FuncAnimation(fig, update_m, interval=FRAME_DELAY,  frames=alpha_iter(total), repeat_delay=REPEAT_DELAY) #builds the animation object, calling the methods and objects defined previously
        
        mywriter = animation.ImageMagickFileWriter(fps=15)

        try:
            g_ani.save( OUTFILE, writer='imagemagick') # saves the animation
        except:
            print("Error aving gif, ensure ImageMagick and its .gif format is installed")
            raise
 
