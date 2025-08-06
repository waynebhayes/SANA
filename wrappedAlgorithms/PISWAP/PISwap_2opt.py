from matlab import WriteMatrix
import networkx as nx
from matching import *

def parseScores(filename):
    # parses a file of scores
    # prepares files for a sparse MATLAB matrix
    f = open(filename,"r")
    g = f.readlines()
    f.close()
    g = [filter(None, x.strip().split(' ')) for x in g]
    names1 = [x[0] for x in g]
    names2 = [x[1] for x in g]
    values = [factor*float(x[2]) for x in g]
    Names1 = list(set(names1))
    Names2 = list(set(names2))
    I = [Names1.index(x) for x in names1]
    J = [Names2.index(x) for x in names2]
    ind = filename.rfind('.')
    core = filename[:ind]
    WriteMatrix([I], core+"I.txt")
    WriteMatrix([J], core+"J.txt")
    WriteMatrix([values], core+"S.txt")
    return (Names1, Names2)

def graphScores(filename, factor = 1, Type = lambda x:float(x)):
    # parses a file of scores and then
    # creates a weighted graph (NetworkX)
    f = open(filename,"r")
    g = f.readlines()
    f.close()
    g = [filter(None, x.strip().split()) for x in g]
    names1 = [x[0] for x in g]
    names2 = [x[1] for x in g]
    values = [factor*float(x[2]) for x in g]
    G = nx.Graph()
    for i in range(len(values)):
        G.add_edge(names1[i], names2[i], Type(values[i]))
    return G

def getGraph(filename):
    # parse a file of interactions and
    # creates a corresponding graph (NetworkX)
    f = open(filename,"r")
    g = f.readlines()
    f.close()
    g = [filter(None, x.strip().split()) for x in g]
    g = g[1:]   # the first line is "Interactor A, interactor B"
    G = nx.Graph()
    for i in range(len(g)):
        G.add_edge(g[i][0], g[i][1])
    Q = G.selfloop_edges()
    if Q:
        print("Removing " + str(len(Q)) + " self-loops")
        G.remove_edges_from(Q)
    return G

def mainAlgo(G1, G2, Gs, M0, alpha, c, Type = float, override = False, factor = 2):
    # computes a matching by the Chindelevitch-Shou algorithm
    # NOTE: assumes that the graph G1 has fewer edges than G2!
    # preprocessing: identify nodes belonging to each species
    V = Gs.nodes()
    sp1 = G1.nodes()[0][:2]
    sp2 = G2.nodes()[0][:2]
    for node in V:
        if not (node in G1.nodes() or node in G2.nodes()):
            # identify the node by the first two letters (temporary hack)
            if node.startswith(sp1):
                G1.add_node(node)
            elif node.startswith(sp2):
                G2.add_node(node)
            else:
                print("Failed to identify correct species for " + node + "!")
    print("Completed node identification")
    V1 = G1.nodes()
    N1 = G1.number_of_nodes()
    V2 = G2.nodes()
    N2 = G2.number_of_nodes()
    X = {}  # holds the indices keyed by the nodes of G1
    for i in range(N1):
        X[V1[i]] = i     
    Y = {}  # holds the indices keyed by the nodes of G2
    for j in range(N2):
        Y[V2[j]] = j
    print("Completed preprocessing")
    print("There are currently " + str(N1) + " nodes in G1 and " + str(N2) + " in G2")
    # first step: compute the preferences of each node
    preferX = {}
    preferY = {}
    for node in X:
        if Gs.has_node(node):
            order = sorted(Gs.neighbors(node), key = lambda x:Gs.get_edge(node,x), reverse = True)
            # get the neighbors of the node in decreasing order of edge weights
            preferY[node] = order[:c]
    for node in Y:
        if Gs.has_node(node):
            order = sorted(Gs.neighbors(node), key = lambda x:Gs.get_edge(node,x), reverse = True)
            # get the neighbors of the node in decreasing order of edge weights
            preferX[node] = order[:c]
    print("Completed step 1")
    (s,t) = findObjective(G1, G2, Gs, M0)
    print("Initial S-value is " + str(s))
    print("Initial T-value is " + str(t))
    if override:
        ratio = float(s*factor)/t
        alpha = ratio/(1+ratio)
        print("Overriding the initial settings: new value of alpha is " + str(alpha)) 
    beta = 1 - alpha # to simplify calculations
    w = alpha*t + beta*s                    # lowercase w denotes the current weight of the matching
    print("Initial W-value is " + str(w))
    # second step: compute the values of s(e), t(e) and w(e) of each edge
    E = [(i, M0[i]) for i in M0 if i in X]  # the set of edges in the matching
    S = zeros([N1,N2], Type)   # contains the s-values (floats by default)
    for edge in Gs.edges():
        (j,k) = edge
        val = Gs.get_edge(j,k)
        if j in X:
            S[X[j],Y[k]] = val
        else:
            S[X[k],Y[j]] = val
    T = zeros([N1,N2], int)   # contains the t-values
    for edge in E:
        (i,j) = edge
        for u in G1.neighbors(i):
            for v in G2.neighbors(j):
                T[X[u], Y[v]] += 1
    print("Completed step 2")
    Swap = {} # contains the possible edges to swap with, along with their values
    for edge in E:
        Swap[edge] = []
        (x,y) = edge
        for u in preferX[y]:
            if u in M0:
                v = M0[u]
                if v in preferY[x]: # check swap of (x,y) with (u,v)
                    xi = X[x]
                    ui = X[u]
                    yi = Y[y]
                    vi = Y[v]
                    loss = beta*(S[xi,yi] + S[ui,vi]) + 2*alpha*(T[xi,yi] + T[ui,vi])
                    gain = beta*(S[xi,vi] + S[ui,yi]) + 2*alpha*(T[xi,vi] + T[ui,yi])
                    if gain > loss:
                        Swap[x,y].append([(u,v), gain-loss])
        Swap[edge].sort(key = lambda z:z[1], reverse=True)
        # make sure that every entry is sorted by the swap values!
    M = {}  # create
    for i in M0:
        M[i] = M0[i]
    print("Completed step 3")
    niter = 0
    bestSwaps = [[z]+Swap[z][0] for z in Swap if Swap[z]]
    while(bestSwaps):
        # update the number of iterations
        niter += 1
        bestSwap = max(bestSwaps, key = lambda z:z[2])
        print("The objective function increased by " + str(bestSwap[2]) + " at iteration " + str(niter))
        # update the total weight of the matching
        w += bestSwap[2]
        edge0 = bestSwap[0]
        (x,y) = edge0
        edge1 = bestSwap[1]
        (u,v) = edge1
        newEdge0 = (x,v)
        newEdge1 = (u,y)
        print("Swapping edges " + str(edge0) + " and " + str(edge1))
        # updating the matching
        M[x] = v
        M[v] = x
        M[u] = y
        M[y] = u
        # computing the indices
        xi = X[x]
        yi = Y[y]
        ui = X[u]
        vi = Y[v]
        # updating E
        E.remove(edge0)
        E.remove(edge1)
        E.append(newEdge0)
        E.append(newEdge1)
        # updating T
        for n1 in G1.neighbors(x):
            for n2 in G2.neighbors(y):
                T[X[n1],Y[n2]] -= 1
            for n2 in G2.neighbors(v):
                T[X[n1],Y[n2]] += 1
        for n1 in G1.neighbors(u):
            for n2 in G2.neighbors(v):
                T[X[n1],Y[n2]] -= 1
            for n2 in G2.neighbors(y):
                T[X[n1],Y[n2]] += 1
        # updating Swap
        del Swap[edge0]
        del Swap[edge1]
        for edge in E:
            if not (edge in [newEdge0, newEdge1]):
                (i,j) = edge
                isx = x in preferX[j]
                isy = y in preferY[i]
                isu = u in preferX[j]
                isv = v in preferY[i]
                if (isx and isy) or (isu and isv):
                    miniSwap = [z[0] for z in Swap[i,j]]
                    if (x,y) in miniSwap:
                        del Swap[i,j][miniSwap.index((x,y))]
                        miniSwap.remove((x,y))  # ensures consistency
                    if (u,v) in miniSwap:
                        del Swap[i,j][miniSwap.index((u,v))]
                if (isx and isv):   # check swap of (i,j) with (x,v)
                    ii = X[i]
                    ji = Y[j]
                    loss = beta*(S[ii,ji] + S[xi,vi]) + 2*alpha*(T[ii,ji] + T[xi,vi])
                    gain = beta*(S[ii,vi] + S[xi,ji]) + 2*alpha*(T[ii,vi] + T[xi,ji])
                    if gain > loss:
                        value = gain-loss
                        ind = 0
                        L = len(Swap[i,j])
                        while (ind < L and Swap[i,j][ind][1] > value):
                            ind += 1
                        Swap[i,j].insert(ind, [(x,v), value])   # put the entry in its right place!
                if (isu and isy):   # check swap of (i,j) with (u,y)
                    ii = X[i]
                    ji = Y[j]
                    loss = beta*(S[ii,ji] + S[ui,yi]) + 2*alpha*(T[ii,ji] + T[ui,yi])
                    gain = beta*(S[ii,yi] + S[ui,ji]) + 2*alpha*(T[ii,yi] + T[ui,ji])
                    if gain > loss:
                        value = gain-loss
                        ind = 0
                        L = len(Swap[i,j])
                        while (ind < L and Swap[i,j][ind][1] > value):
                            ind += 1
                        Swap[i,j].insert(ind, [(u,y), value])   # put the entry in its right place! 
            elif edge == newEdge0:
                if edge in Swap:
                    print("Error: an edge was found in Swap which should never be there!")
                Swap[edge] = []
                for i in preferX[v]:
                    if i in M:
                        j = M[i]
                        if j in preferY[x]:     # check swap of (x,v) with (i,j)
                            ii = X[i]
                            ji = Y[j]
                            loss = beta*(S[ii,ji] + S[xi,vi]) + 2*alpha*(T[ii,ji] + T[xi,vi])
                            gain = beta*(S[ii,vi] + S[xi,ji]) + 2*alpha*(T[ii,vi] + T[xi,ji])
                            if gain > loss:
                                Swap[x,v].append([(i,j), gain-loss])
                Swap[edge].sort(key = lambda z:z[1], reverse=True)
            elif edge == newEdge1:
                if edge in Swap:
                    print("Error: an edge was found in Swap which should never be there!")
                Swap[edge] = []
                for i in preferX[y]:
                    if i in M:
                        j = M[i]
                        if j in preferY[u]:     # check swap of (u,y) with (i,j)
                            ii = X[i]
                            ji = Y[j]
                            loss = beta*(S[ii,ji] + S[ui,yi]) + 2*alpha*(T[ii,ji] + T[ui,yi])
                            gain = beta*(S[ii,yi] + S[ui,ji]) + 2*alpha*(T[ii,yi] + T[ui,ji])
                            if gain > loss:
                                Swap[u,y].append([(i,j), gain-loss])
                Swap[edge].sort(key = lambda z:z[1], reverse=True)
            else:
                print("This should never happen!")
        # updating bestSwaps
        bestSwaps = [[z]+Swap[z][0] for z in Swap if Swap[z]]
    return (w, M)   # no further improvement is possible!

def findObjective(G1, G2, Gs, M0):
    # computes the value of the objective function:
    # S = total weight of matching, T = # conserved edges
    (S,T)=(0,0)
    for i in M0:
        if i in G1:
            j = M0[i]
            S+=Gs.get_edge(i,j)
            for u in G1.neighbors(i):
                if u in M0:
                    v = M0[u]
                    if G2.has_edge(j,v):
                        T+=1
    return (S,T)
    
def processOnce(graph1, graph2, graphS, alpha, c):
    # wrapper for a single run of the main algorithm
    if graph1.number_of_edges() > graph2.number_of_edges():
        return mainAlgo(graph2, graph1, graphS, alpha, c)
    else:
        return mainAlgo(graph1, graph2, graphS, alpha, c)

def findHomologene(Homologs0, Homologs1, Synonyms, mapping):
    # finds all homologs in a mapping based on a synonym dictionary
    transMap = {}
    for i in mapping:
        if i in Synonyms:
            keys = Synonyms[i]
            j = mapping[i]
            vals = Synonyms[j]
            for key in keys:
                if key in transMap:
                    print("Error: the key " + str(key) + " is already present!")
                else:
                    transMap[key] = vals
    count = 0
    match = {}
    for i in transMap:
        if i in Homologs0:
            clus0 = Homologs0[i]
            j = transMap[i]
            for k in j:
                if k in Homologs1:
                    clus1 = Homologs1[k]
                    if clus0 == clus1:
                        count += 1
                        match[i] = k
    print(count)
    return (match, transMap)

def findGO(filename):
    f = open(filename)
    G = f.readlines()
    f.close()
    G=filter(lambda x:not x.startswith('!'),G)
    G=[x.strip() for x in G]
    G=[x.split('\t') for x in G]
    G=filter(lambda x:not x[6] in ['ISS','IEA','ND'],G)
    H=[[x[2],x[10],x[4]] for x in G]
    K=[[list(set([x[0]]+x[1].split('|'))), x[2]] for x in H]
    Dic = {}
    for x in K:
        for y in x[0]:
                if y in Dic:
                    Dic[y].append(x[1])
                else:
                    Dic[y] = [x[1]]
    return Dic

def mapToStandard(GODict, standDict):
    out = {}
    for i in GODict:
        lis = [standDict[j] for j in GODict[i] if j in standDict]
        out[i] = sum(lis,[])
        if not out[i]:
            del out[i]
    return out

def mapReverse(mappings):
    revMap= {}
    for mapping in mappings:
        for key in mapping:
            vals = mapping[key]
            for val in vals:
                if val in revMap:
                    revMap[val].append(key)
                else:
                    revMap[val] = [key]
    for i in revMap:
        revMap[i] = list(set(revMap[i]))
    return revMap

def computeFC(mapping, goMap0, goMap1):
    revMap = mapReverse((goMap0,goMap1))
    sims = {}
    for keys in mapping:
        vals = mapping[keys]
        Set = []
        in0 = 0
        for key in keys:
            if key in goMap0:
                in0 += 1
                Set += goMap0[key]
        in1 = 0
        for val in vals:
            if val in goMap1:
                in1 += 1
                Set += goMap1[val]
        Set = list(set(Set))
        if in0 and in1:
            total = []
            for i in range(len(Set)):
                t1 = Set[i]
                S1 = set(revMap[t1])
                for j in range(i):
                    t2 = Set[j]
                    S2 = revMap[t2]
                    total.append(float(len(S1.intersection(S2)))/len(S1.union(S2)))
            total.sort(reverse=True)
            sims[keys] = total[len(total)/2]
    return sims
