#from matlab import WriteMatrix
import networkx as nx
from matching import *
from numpy import *
#def parseScores(filename):
    # parses a file of scores
    # prepares files for a sparse MATLAB matrix
 #   f = open(filename,"r")
 #   g = f.readlines()
 #   f.close()
 #   g = [filter(None, x.strip().split(' ')) for x in g]
 #   names1 = [x[0] for x in g]
 #   names2 = [x[1] for x in g]
 #   values = [factor*float(x[2]) for x in g]
 #   Names1 = list(set(names1))
 #   Names2 = list(set(names2))
 #   I = [Names1.index(x) for x in names1]
 #   J = [Names2.index(x) for x in names2]
 #   ind = filename.rfind('.')
 #   core = filename[:ind]
 #   WriteMatrix([I], core+"I.txt")
 #   WriteMatrix([J], core+"J.txt")
 #   WriteMatrix([values], core+"S.txt")
 #   return (Names1, Names2)

def graphScores(filename, factor = 1, Type = lambda x:float(x)):
    # parses a file of scores and then
    # creates a weighted graph (NetworkX)
    print("Get graphScore ...")
    f = open(filename,"r")
    g = f.readlines()
    f.close()
    g = [filter(None, x.strip().split()) for x in g]
    names1 = [x[0] for x in g]
    names2 = [x[1] for x in g]
    values = [factor*float(x[2]) for x in g]
    G = nx.Graph()
    for i in range(len(values)):
        G.add_edge(names1[i], names2[i],{'weight':values[i]})
        #print(names1[i] + " " + names2[i] + " weight: " + str(values[i]))
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

def mainAlgo(G1, G2, Gs, M0, alpha, c, Type = float, override = True, factor = 2):
    # computes a matching by the Chindelevitch-Shou algorithm
    # NOTE: assumes that the graph G1 has fewer edges than G2!
    # preprocessing: identify nodes belonging to each species
    print("Start main algorithm...")
    count = 0
    for i in M0:
        count += 1
    print("edge: " + str(count))
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
            order = sorted(Gs.neighbors(node), key = lambda x:Gs.edge[node][x]['weight'], reverse = True)
            # get the neighbors of the node in decreasing order of edge weights
            preferY[node] = order[:c]
    for node in Y:
        if Gs.has_node(node):
            order = sorted(Gs.neighbors(node), key = lambda x:Gs.edge[node][x]['weight'], reverse = True)
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
    print("Alpha =" + str(alpha))
    beta = 1 - alpha # to simplify calculations
    w = alpha*t + beta*s                    # lowercase w denotes the current weight of the matching
    print("Initial W-value is " + str(w))
    # second step: compute the values of s(e), t(e) and w(e) of each edge
    E = [(i, M0[i]) for i in M0 if i in X]  # the set of edges in the matching
    S = empty([N1,N2], Type)   # contains the s-values (floats by default)
    for edge in Gs.edges():
        (j,k) = edge
        val = Gs.edge[j][k]['weight']
        if j in X and k in Y:
            S[X[j],Y[k]] = val
        else:
            S[X[k],Y[j]] = val
    T = empty([N1,N2], int)   # contains the t-values
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
        xi = X[x]
        yi = Y[y]
        try: preferX[y]
        except KeyError:
                continue
        try: preferY[x]
        except KeyError:
                continue
        for u in preferX[y]:
            if u in M0 and u != x:
                v = M0[u]
                if v in preferY[x]:
                    ui = X[u]
                    vi = Y[v]
                    loss2_1 = (beta*(S[xi,yi] + S[ui,vi]) + 2*alpha*(T[xi,yi] + T[ui,vi]))/2
                    gain2_1 = (beta*(S[xi,vi] + S[ui,yi]) + 2*alpha*(T[xi,vi] + T[ui,yi]))/2
                    if gain2_1 > loss2_1:
                        Swap[x,y].append([(x,y),(u,v),0,(x,v),(u,y),0,gain2_1 - loss2_1])
                    for p in preferX[y]:
                        if p in M0 and p != u and p != x:
                            q = M0[p]
                            if q in preferY[x]:
                                pi = X[p]
                                qi = Y[q]
                                loss3 = (beta*(S[xi,yi] + S[ui,vi] + S[pi,qi]) + 2*alpha*(T[xi,yi] + T[ui,vi] + T[pi,qi]))/3
                                gain3_1 = (beta*(S[xi,qi] + S[ui,yi] + S[pi,vi]) + 2*alpha*(T[xi,qi] + T[ui,yi] + T[pi,vi]))/3
                                #gain3_2 = beta*(S[xi,vi] + S[ui,qi] + S[pi,yi]) + 2*alpha*(T[xi,vi] + T[ui,qi] + T[pi,yi])
                                #loss2_2 = beta*(S[xi,yi] + S[pi,qi]) + 2*alpha*(T[xi,yi] + T[pi,qi])
                                #gain2_2 = beta*(S[xi,qi] + S[pi,yi]) + 2*alpha*(T[xi,qi] + T[pi,yi])
                                if gain3_1 > loss3:
                                    Swap[x,y].append([(x,y),(u,v),(p,q),(x,q),(u,y),(p,v),gain3_1 - loss3])
                                #if gain3_2 > loss3:
                                #    Swap[x,y].append([(x,y),(u,v),(p,q),(x,v),(u,q),(p,y),gain3_2 - loss3])
                                #if gain2_2 > loss2_2:
                                #    Swap[x,y].append([(x,y),(p,q),0,(x,q),(p,y),0,gain2_2 - loss2_2])
        Swap[edge].sort(key = lambda z:z[6], reverse=True)                        
    M = {}  # create
    for i in M0:
        M[i] = M0[i]
    print("Completed step 3")
    niter = 0
    bestSwaps = [Swap[z][0] for z in Swap if Swap[z]]
    temp =["","","","","","","","","",""]
    #temp[0] = ""
    #temp[1] = ""
    flag = 0; 
    while(bestSwaps):
        # update the number of iterations
        #brainma721
        if niter > 550:
		    break
        niter += 1
        bestSwap = max(bestSwaps, key = lambda z:z[6])
        print("The objective function increased by " + str(bestSwap[6]) + " at iteration " + str(niter))
        # update the total weight of the matching
        w += bestSwap[6]
        if bestSwap[2] == 0:
            edge0 = bestSwap[0]
            (x,y) = edge0
            edge1 = bestSwap[1]
            (u,v) = edge1
            newEdge0 = (x,v)
            newEdge1 = (u,y)
            current_swap = "Swapping edges " + str(edge0) + " and " + str(edge1)
            if current_swap in temp:
                flag += 1
            else:
                flag = 0
            if flag >= 2:
                break
            for index in range(9):
                temp[index] = temp[index+1]
            #if flag == 0 and current_swap == temp[1]:
                #flag+=1
            #elif flag == 1 and current_swap == temp[1]:
                #flag+=1
                #break
            #elif flag == 1 and current_swap != temp[1]:
                #flag = 0
            #temp[1] = temp[0]
            temp[9] = current_swap
            print(current_swap)
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
                    try: preferX[j]
                    except KeyError:
                        continue
                    try: preferY[i]
                    except KeyError:
                        continue
                    isx = x in preferX[j]
                    isy = y in preferY[i]
                    isu = u in preferX[j]
                    isv = v in preferY[i]
                    if (isx and isy) or (isu and isv):
                        #miniSwap = [z[0]+z[1] for z in Swap[i,j]]
                        tSwap =[]
                        for z in range(len(Swap[i,j])):
                            tSwap.append(Swap[i,j][z])
                        for miniSwap in tSwap:
                            if ((x,y) in miniSwap) or ((u,v) in miniSwap):
                                del Swap[i,j][Swap[i,j].index(miniSwap)]
                                #miniSwap.remove((x,y))  # ensures consistency
                            
                    if (isx and isv):   # check swap of (i,j) with (x,v)
                        ii = X[i]
                        ji = Y[j]
                        loss2_1 = (beta*(S[ii,ji] + S[xi,vi]) + 2*alpha*(T[ii,ji] + T[xi,vi]))/2
                        gain2_1 = (beta*(S[ii,vi] + S[xi,ji]) + 2*alpha*(T[ii,vi] + T[xi,ji]))/2
                        if gain2_1 > loss2_1:
                            L = len(Swap[i,j])
                            ind = 0
                            value = gain2_1 - loss2_1
                            while (ind < L and Swap[i,j][ind][6] > value):
                                ind += 1
                            Swap[i,j].insert(ind, [(i,j),(x,v),0,(i,v),(x,j),0,value])
                        for p in preferX[j]:
                            if p in M and p != x and p != i:
                                q = M[p]
                                if q in preferY[i]:
                                    pi = X[p]
                                    qi = Y[q]
                                    loss3 = (beta*(S[ii,ji] + S[xi,vi] + S[pi,qi]) + 2*alpha*(T[ii,ji] + T[xi,vi] + T[pi,qi]))/3
                                    gain3_1 = (beta*(S[ii,qi] + S[xi,ji] + S[pi,vi]) + 2*alpha*(T[ii,qi] + T[xi,ji] + T[pi,vi]))/3
                                    gain3_2 = (beta*(S[ii,vi] + S[xi,qi] + S[pi,ji]) + 2*alpha*(T[ii,vi] + T[xi,qi] + T[pi,ji]))/3
                                    
                                    if gain3_1 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_1 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(x,v),(p,q),(i,q),(x,j),(p,v),value])
                                    if gain3_2 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_2 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(x,v),(p,q),(i,v),(x,q),(p,j),value])
                                     # put the entry in its right place!
                    if (isu and isy):   # check swap of (i,j) with (u,y)
                        ii = X[i]
                        ji = Y[j]
                        loss2_1 = (beta*(S[ii,ji] + S[ui,yi]) + 2*alpha*(T[ii,ji] + T[ui,yi]))/2
                        gain2_1 = (beta*(S[ii,yi] + S[ui,ji]) + 2*alpha*(T[ii,yi] + T[ui,ji]))/2
                        if gain2_1 > loss2_1:
                            L = len(Swap[i,j])
                            ind = 0
                            value = gain2_1 - loss2_1
                            while (ind < L and Swap[i,j][ind][6] > value):
                                ind += 1
                            Swap[i,j].insert(ind, [(i,j),(u,y),0,(i,y),(u,j),0,value])
                        for p in preferX[j]:
                            if p in M and p != u and p != i:
                                q = M[p]
                                if q in preferY[i]:
                                    pi = X[p]
                                    qi = Y[q]
                                    loss3 = (beta*(S[ii,ji] + S[ui,yi] + S[pi,qi]) + 2*alpha*(T[ii,ji] + T[ui,yi] + T[pi,qi]))/3
                                    gain3_1 = (beta*(S[ii,qi] + S[ui,ji] + S[pi,yi]) + 2*alpha*(T[ii,qi] + T[ui,ji] + T[pi,yi]))/3
                                    gain3_2 = (beta*(S[ii,yi] + S[ui,qi] + S[pi,ji]) + 2*alpha*(T[ii,yi] + T[ui,qi] + T[pi,ji]))/3
                                    
                                    if gain3_1 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_1 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(u,y),(p,q),(i,q),(u,j),(p,y),value])
                                    if gain3_2 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_2 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(u,y),(p,q),(i,y),(u,q),(p,j),value])
                                     # put the entry in its right place!
                elif edge == newEdge0:
                    if edge in Swap:
                        print("Error: an edge was found in Swap which should never be there!")
                    Swap[edge] = []
                    xi = X[x]
                    vi = Y[v]
                    for i in preferX[v]:
                        if i in M and i != x:
                            j = M[i]
                            if j in preferY[x]:     # check swap of (x,v) with (i,j)
                                ii = X[i]
                                ji = Y[j]
                                loss2_1 = (beta*(S[xi,vi] + S[ii,ji]) + 2*alpha*(T[xi,vi] + T[ii,ji]))/2
                                gain2_1 = (beta*(S[xi,ji] + S[ii,vi]) + 2*alpha*(T[xi,ji] + T[ii,vi]))/2
                                if gain2_1 > loss2_1:
                                    Swap[x,v].append([(x,v),(i,j),0,(x,j),(i,v),0,gain2_1 - loss2_1])
                                for p in preferX[v]:
                                    if p in M and p != i and p != x:
                                        q = M[p]
                                        if q in preferY[x]:
                                            pi = X[p]
                                            qi = Y[q]
                                            loss3 = (beta*(S[xi,vi] + S[ii,ji] + S[pi,qi]) + 2*alpha*(T[xi,vi] + T[ii,ji] + T[pi,qi]))/3
                                            gain3_1 = (beta*(S[xi,qi] + S[ii,vi] + S[pi,ji]) + 2*alpha*(T[xi,qi] + T[ii,vi] + T[pi,ji]))/3
                                            #gain3_2 = beta*(S[xi,ji] + S[ii,qi] + S[pi,vi]) + 2*alpha*(T[xi,ji] + T[ii,qi] + T[pi,vi])
                                            #loss2_2 = beta*(S[xi,vi] + S[pi,qi]) + 2*alpha*(T[xi,vi] + T[pi,qi])
                                            #gain2_2 = beta*(S[xi,qi] + S[pi,vi]) + 2*alpha*(T[xi,qi] + T[pi,vi])
                                            if gain3_1 > loss3:
                                                Swap[x,v].append([(x,v),(i,j),(p,q),(x,q),(i,v),(p,j),gain3_1 - loss3])
                                            #if gain3_2 > loss3:
                                            #    Swap[x,v].append([(x,v),(i,j),(p,q),(x,j),(i,q),(p,v),gain3_2 - loss3])
                                            #if gain2_2 > loss2_2:
                                            #    Swap[x,v].append([(x,v),(p,q),0,(x,q),(p,v),0,gain2_2 - loss2_2])
                    Swap[edge].sort(key = lambda z:z[6], reverse=True)
                                
                elif edge == newEdge1:
                    if edge in Swap:
                        print("Error: an edge was found in Swap which should never be there!")
                    Swap[edge] = []
                    ui = X[u]
                    yi = Y[y]
                    for i in preferX[y]:
                        if i in M and i != u:
                            j = M[i]
                            if j in preferY[u]:     # check swap of (x,v) with (i,j)
                                ii = X[i]
                                ji = Y[j]
                                loss2_1 = (beta*(S[ui,yi] + S[ii,ji]) + 2*alpha*(T[ui,yi] + T[ii,ji]))/2
                                gain2_1 = (beta*(S[ui,ji] + S[ii,yi]) + 2*alpha*(T[ui,ji] + T[ii,yi]))/2
                                if gain2_1 > loss2_1:
                                    Swap[u,y].append([(u,y),(i,j),0,(u,j),(i,y),0,gain2_1 - loss2_1])
                                for p in preferX[y]:
                                    if p in M and p != i and p != u:
                                        q = M[p]
                                        if q in preferY[u]:
                                            pi = X[p]
                                            qi = Y[q]
                                            loss3 = (beta*(S[ui,yi] + S[ii,ji] + S[pi,qi]) + 2*alpha*(T[ui,yi] + T[ii,ji] + T[pi,qi]))/3
                                            gain3_1 = (beta*(S[ui,qi] + S[ii,yi] + S[pi,ji]) + 2*alpha*(T[ui,qi] + T[ii,yi] + T[pi,ji]))/3
                                            #gain3_2 = (beta*(S[ui,ji] + S[ii,qi] + S[pi,yi]) + 2*alpha*(T[ui,ji] + T[ii,qi] + T[pi,yi]))/3
                                            #loss2_2 = beta*(S[ui,yi] + S[pi,qi]) + 2*alpha*(T[ui,yi] + T[pi,qi])
                                            #gain2_2 = beta*(S[ui,qi] + S[pi,yi]) + 2*alpha*(T[ui,qi] + T[pi,yi])
                                            if gain3_1 > loss3:
                                                Swap[u,y].append([(u,y),(i,j),(p,q),(u,q),(i,y),(p,j),gain3_1 - loss3])
                                            #if gain3_2 > loss3:
                                            #    Swap[u,y].append([(u,y),(i,j),(p,q),(u,j),(i,q),(p,y),gain3_2 - loss3])
                                            #if gain2_2 > loss2_2:
                                            #    Swap[u,y].append([(u,y),(p,q),0,(u,q),(p,y),0,gain2_2 - loss2_2])
                    Swap[edge].sort(key = lambda z:z[6], reverse=True)
                else:
                    print("This should never happen!")
            # updating bestSwaps
        elif bestSwap[2] != 0:
            edge0 = bestSwap[0]
            (x,y) = edge0
            edge1 = bestSwap[1]
            (u,v) = edge1
            edge2 = bestSwap[2]
            (p,q) = edge2
            newEdge0 = bestSwap[3]
            (x,ny) = newEdge0
            newEdge1 = bestSwap[4]
            (u,nv) = newEdge1 
            newEdge2 = bestSwap[5]
            (p,nq) = newEdge2
            current_swap = "Swapping edges " + str(edge0) + " and " + str(edge1)+ "and" + str(edge2) +"=>"+ str(newEdge0)+ "and" +str (newEdge1) + "and" +str(newEdge2)
            if current_swap in temp:
                flag += 1
            else:
                flag = 0
            if flag >= 2:
                break
            for index in range(9):
                temp[index] = temp[index+1]
            #if flag == 0 and current_swap == temp[1]:
                #flag+=1
            #elif flag == 1 and current_swap == temp[1]:
                #flag+=1
                #break
            #elif flag == 1 and current_swap != temp[1]:
                #flag = 0
            #temp[1] = temp[0]
            temp[9] = current_swap
            print(current_swap)
            # updating the matching
            M[x] = ny
            M[ny] = x
            M[u] = nv
            M[nv] = u
            M[p] = nq
            M[nq] = p
            # computing the indices
            xi = X[x]
            yi = Y[y]
            ui = X[u]
            vi = Y[v]
            pi = X[p]
            qi = Y[q]
            nyi = Y[ny]
            nvi = Y[nv]
            nqi = Y[nq]
            # updating E
            E.remove(edge0)
            E.remove(edge1)
            E.remove(edge2)
            E.append(newEdge0)
            E.append(newEdge1)
            E.append(newEdge2)
            # updating T
            for n1 in G1.neighbors(x):
                for n2 in G2.neighbors(y):
                    T[X[n1],Y[n2]] -= 1
                for n2 in G2.neighbors(ny):
                    T[X[n1],Y[n2]] += 1
            for n1 in G1.neighbors(u):
                for n2 in G2.neighbors(v):
                    T[X[n1],Y[n2]] -= 1
                for n2 in G2.neighbors(nv):
                    T[X[n1],Y[n2]] += 1
            for n1 in G1.neighbors(p):
                for n2 in G2.neighbors(q):
                    T[X[n1],Y[n2]] -= 1
                for n2 in G2.neighbors(nq):
                    T[X[n1],Y[n2]] += 1
            # updating Swap
            del Swap[edge0]
            del Swap[edge1]
            del Swap[edge2]
            for edge in E:
                if not (edge in [newEdge0, newEdge1, newEdge2]):
                    (i,j) = edge
                    try: preferX[j]
                    except KeyError:
                        continue
                    try: preferY[i]
                    except KeyError:
                        continue
                    isx = x in preferX[j]
                    isy = y in preferY[i]
                    isu = u in preferX[j]
                    isv = v in preferY[i]
                    isp = p in preferX[j]
                    isq = q in preferY[i]
                    isny = ny in preferY[i]
                    isnv = nv in preferY[i]
                    isnq = nq in preferY[i]
                    if (isx and isy) or (isu and isv) or (isp and isq):
                        #print ("(i,j)=(" + i + "," + j +")")
                        #print ("Swap[i,j]=" + str(Swap[i,j]))
                        tSwap =[]
                        for z in range(len(Swap[i,j])):
                            tSwap.append(Swap[i,j][z])
                        for miniSwap in tSwap:
                            #print("miniSwap=" + str(miniSwap))
                            #print ("Swap[i,j]=" + str(Swap[i,j]))
                            #print ("tSwap=" + str(tSwap))
                            #print ("Swap[i,j].index(miniSwap)=" + str(Swap[i,j].index(miniSwap)))
                            if ((x,y) in miniSwap) or ((u,v) in miniSwap) or ((p,q) in miniSwap):
                                #print ("delete:" + str(Swap[i,j][Swap[i,j].index(miniSwap)]))
                                del Swap[i,j][Swap[i,j].index(miniSwap)]
                                #print ("Swap[i,j]=" + str(Swap[i,j]))
                                #print ("tSwap=" + str(tSwap))
                                #miniSwap.remove((x,y))  # ensures consistency
                            
                    if (isx and isny):   # check swap of (i,j) with (x,v)
                        ii = X[i]
                        ji = Y[j]
                        loss2_1 = (beta*(S[ii,ji] + S[xi,nyi]) + 2*alpha*(T[ii,ji] + T[xi,nyi]))/2
                        gain2_1 = (beta*(S[ii,nyi] + S[xi,ji]) + 2*alpha*(T[ii,nyi] + T[xi,ji]))/2
                        if gain2_1 > loss2_1:
                            L = len(Swap[i,j])
                            ind = 0
                            value = gain2_1 - loss2_1
                            while (ind < L and Swap[i,j][ind][6] > value):
                                ind += 1
                            Swap[i,j].insert(ind, [(i,j),(x,ny),0,(i,ny),(x,j),0,value])
                        for a in preferX[j]:
                            if a in M and a != x and a != i:
                                s = M[a]
                                if s in preferY[i]:
                                    ai = X[a]
                                    si = Y[s]
                                    loss3 = (beta*(S[ii,ji] + S[xi,nyi] + S[ai,si]) + 2*alpha*(T[ii,ji] + T[xi,nyi] + T[ai,si]))/3
                                    gain3_1 = (beta*(S[ii,si] + S[xi,ji] + S[ai,nyi]) + 2*alpha*(T[ii,si] + T[xi,ji] + T[ai,nyi]))/3
                                    gain3_2 = (beta*(S[ii,nyi] + S[xi,si] + S[ai,ji]) + 2*alpha*(T[ii,nyi] + T[xi,si] + T[ai,ji]))/3
                                    if gain3_1 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_1 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(x,ny),(a,s),(i,s),(x,j),(a,ny),value])
                                    if gain3_2 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_2 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(x,ny),(a,s),(i,ny),(x,s),(a,j),value])
                                     # put the entry in its right place!
                    if (isu and isnv):   # check swap of (i,j) with (u,y)
                        ii = X[i]
                        ji = Y[j]
                        loss2_1 = (beta*(S[ii,ji] + S[ui,nvi]) + 2*alpha*(T[ii,ji] + T[ui,nvi]))/2
                        gain2_1 = (beta*(S[ii,nvi] + S[ui,ji]) + 2*alpha*(T[ii,nvi] + T[ui,ji]))/2
                        if gain2_1 > loss2_1:
                            L = len(Swap[i,j])
                            ind = 0
                            value = gain2_1 - loss2_1
                            while (ind < L and Swap[i,j][ind][6] > value):
                                ind += 1
                            Swap[i,j].insert(ind, [(i,j),(u,nv),0,(i,nv),(u,j),0,value])
                        for a in preferX[j]:
                            if a in M and a != u and a != i:
                                s = M[a]
                                if s in preferY[i]:
                                    ai = X[a]
                                    si = Y[s]
                                    loss3 = (beta*(S[ii,ji] + S[ui,nvi] + S[ai,si]) + 2*alpha*(T[ii,ji] + T[ui,nvi] + T[ai,si]))/3
                                    gain3_1 = (beta*(S[ii,si] + S[ui,ji] + S[ai,nvi]) + 2*alpha*(T[ii,si] + T[ui,ji] + T[ai,nvi]))/3
                                    gain3_2 = (beta*(S[ii,nvi] + S[ui,si] + S[ai,ji]) + 2*alpha*(T[ii,nvi] + T[ui,si] + T[ai,ji]))/3
                                    if gain3_1 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_1 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(u,nv),(a,s),(i,s),(u,j),(a,nv),value])
                                    if gain3_2 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_2 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(u,nv),(a,s),(i,nv),(u,s),(a,j),value])
                                     # put the entry in its right place! 
                    if (isp and isnq):   # check swap of (i,j) with (u,y)
                        ii = X[i]
                        ji = Y[j]
                        loss2_1 = (beta*(S[ii,ji] + S[pi,nqi]) + 2*alpha*(T[ii,ji] + T[pi,nqi]))/2
                        gain2_1 = (beta*(S[ii,nqi] + S[pi,ji]) + 2*alpha*(T[ii,nqi] + T[pi,ji]))/2
                        if gain2_1 > loss2_1:
                            L = len(Swap[i,j])
                            ind = 0
                            value = gain2_1 - loss2_1
                            while (ind < L and Swap[i,j][ind][6] > value):
                                ind += 1
                            Swap[i,j].insert(ind, [(i,j),(p,nq),0,(i,nq),(p,j),0,value])
                        for a in preferX[j]:
                            if a in M and a != p and a!=i:
                                s = M[a]
                                if s in preferY[i]:
                                    ai = X[a]
                                    si = Y[s]
                                    loss3 = (beta*(S[ii,ji] + S[pi,nqi] + S[ai,si]) + 2*alpha*(T[ii,ji] + T[pi,nqi] + T[ai,si]))/3
                                    gain3_1 = (beta*(S[ii,si] + S[pi,ji] + S[ai,nqi]) + 2*alpha*(T[ii,si] + T[pi,ji] + T[ai,nqi]))/3
                                    gain3_2 = (beta*(S[ii,nqi] + S[pi,si] + S[ai,ji]) + 2*alpha*(T[ii,nqi] + T[pi,si] + T[ai,ji]))/3
                                    if gain3_1 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_1 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(p,nq),(a,s),(i,s),(p,j),(a,nq),value])
                                    if gain3_2 > loss3:
                                        L = len(Swap[i,j])
                                        ind = 0
                                        value = gain3_2 - loss3
                                        while (ind < L and Swap[i,j][ind][6] > value):
                                            ind += 1
                                        Swap[i,j].insert(ind, [(i,j),(p,nq),(a,s),(i,nq),(p,s),(a,j),value])
                                     # put the entry in its right place! 
                elif edge == newEdge0:
                    if edge in Swap:
                        print("Error: an edge was found in Swap which should never be there!")
                    Swap[edge] = []
                    xi = X[x]
                    nyi = Y[ny]
                    for i in preferX[ny]:
                        if i in M and i != x:
                            j = M[i]
                            if j in preferY[x]:     # check swap of (x,v) with (i,j)
                                ii = X[i]
                                ji = Y[j]
                                loss2_1 = (beta*(S[xi,nyi] + S[ii,ji]) + 2*alpha*(T[xi,nyi] + T[ii,ji]))/2
                                gain2_1 = (beta*(S[xi,ji] + S[ii,nyi]) + 2*alpha*(T[xi,ji] + T[ii,nyi]))/2
                                if gain2_1 > loss2_1:
                                    Swap[x,ny].append([(x,ny),(i,j),0,(x,j),(i,ny),0,gain2_1 - loss2_1])
                                for a in preferX[ny]:
                                    if a in M and a != i and a != x:
                                        s = M[a]
                                        if s in preferY[x]:
                                            ai = X[a]
                                            si = Y[s]
                                            loss3 = (beta*(S[xi,nyi] + S[ii,ji] + S[ai,si]) + 2*alpha*(T[xi,nyi] + T[ii,ji] + T[ai,si]))/3
                                            gain3_1 = (beta*(S[xi,si] + S[ii,nyi] + S[ai,ji]) + 2*alpha*(T[xi,si] + T[ii,nyi] + T[ai,ji]))/3
                                            #gain3_2 = beta*(S[xi,ji] + S[ii,si] + S[ai,nyi]) + 2*alpha*(T[xi,ji] + T[ii,si] + T[ai,nyi])
                                            #loss2_2 = beta*(S[xi,nyi] + S[ai,si]) + 2*alpha*(T[xi,nyi] + T[ai,si])
                                            #gain2_2 = beta*(S[xi,si] + S[ai,nyi]) + 2*alpha*(T[xi,si] + T[ai,nyi])
                                            if gain3_1 > loss3:
                                                Swap[x,ny].append([(x,ny),(i,j),(a,s),(x,s),(i,ny),(a,j),gain3_1 - loss3])
                                            #if gain3_2 > loss3:
                                            #    Swap[x,ny].append([(x,ny),(i,j),(a,s),(x,j),(i,s),(a,ny),gain3_2 - loss3])
                                            #if gain2_2 > loss2_2:
                                            #    Swap[x,ny].append([(x,ny),(a,s),0,(x,s),(a,ny),0,gain2_2 - loss2_2])
                    Swap[edge].sort(key = lambda z:z[6], reverse=True)
                elif edge == newEdge1:
                    if edge in Swap:
                        print("Error: an edge was found in Swap which should never be there!")
                    Swap[edge] = []
                    ui = X[u]
                    nvi = Y[nv]
                    for i in preferX[nv]:
                        if i in M and i != u:
                            j = M[i]
                            if j in preferY[u]:     # check swap of (x,v) with (i,j)
                                ii = X[i]
                                ji = Y[j]
                                loss2_1 = (beta*(S[ui,nvi] + S[ii,ji]) + 2*alpha*(T[ui,nvi] + T[ii,ji]))/2
                                gain2_1 = (beta*(S[ui,ji] + S[ii,nvi]) + 2*alpha*(T[ui,ji] + T[ii,nvi]))/2
                                if gain2_1 > loss2_1:
                                    Swap[u,nv].append([(u,nv),(i,j),0,(u,j),(i,nv),0,gain2_1 - loss2_1])
                                for a in preferX[nv]:
                                    if a in M and a != i and a != u:
                                        s = M[a]
                                        if s in preferY[u]:
                                            ai = X[a]
                                            si = Y[s]
                                            loss3 = (beta*(S[ui,nvi] + S[ii,ji] + S[ai,si]) + 2*alpha*(T[ui,nvi] + T[ii,ji] + T[ai,si]))/3
                                            gain3_1 = (beta*(S[ui,si] + S[ii,nvi] + S[ai,ji]) + 2*alpha*(T[ui,si] + T[ii,nvi] + T[ai,ji]))/3
                                            #gain3_2 = beta*(S[ui,ji] + S[ii,si] + S[ai,nvi]) + 2*alpha*(T[ui,ji] + T[ii,si] + T[ai,nvi])
                                            #loss2_2 = beta*(S[ui,nvi] + S[ai,si]) + 2*alpha*(T[ui,nvi] + T[ai,si])
                                            #gain2_2 = beta*(S[ui,si] + S[ai,nvi]) + 2*alpha*(T[ui,si] + T[ai,nvi])
                                            if gain3_1 > loss3:
                                                Swap[u,nv].append([(u,nv),(i,j),(a,s),(u,s),(i,nv),(a,j),gain3_1 - loss3])
                                            #if gain3_2 > loss3:
                                            #    Swap[u,nv].append([(u,nv),(i,j),(a,s),(u,j),(i,s),(a,nv),gain3_2 - loss3])
                                            #if gain2_2 > loss2_2:
                                            #    Swap[u,nv].append([(u,nv),(a,s),0,(u,s),(a,nv),0,gain2_2 - loss2_2])
                    Swap[edge].sort(key = lambda z:z[6], reverse=True)
                elif edge == newEdge2:
                    if edge in Swap:
                        print("Error: an edge was found in Swap which should never be there!")
                    Swap[edge] = []
                    pi = X[p]
                    nqi = Y[nq]
                    for i in preferX[nq]:
                        if i in M and i != p:
                            j = M[i]
                            if j in preferY[p]:     # check swap of (x,v) with (i,j)
                                ii = X[i]
                                ji = Y[j]
                                loss2_1 = (beta*(S[pi,nqi] + S[ii,ji]) + 2*alpha*(T[pi,nqi] + T[ii,ji]))/2
                                gain2_1 = (beta*(S[pi,ji] + S[ii,nqi]) + 2*alpha*(T[pi,ji] + T[ii,nqi]))/2
                                if gain2_1 > loss2_1:
                                    Swap[p,nq].append([(p,nq),(i,j),0,(p,j),(i,nq),0,gain2_1 - loss2_1])
                                for a in preferX[nq]:
                                    if a in M and a != i and a != p:
                                        s = M[a]
                                        if s in preferY[p]:
                                            ai = X[a]
                                            si = Y[s]
                                            loss3 = (beta*(S[pi,nqi] + S[ii,ji] + S[ai,si]) + 2*alpha*(T[pi,nqi] + T[ii,ji] + T[ai,si]))/3
                                            gain3_1 = (beta*(S[pi,si] + S[ii,nqi] + S[ai,ji]) + 2*alpha*(T[pi,si] + T[ii,nqi] + T[ai,ji]))/3
                                            #gain3_2 = beta*(S[pi,ji] + S[ii,si] + S[ai,nqi]) + 2*alpha*(T[pi,ji] + T[ii,si] + T[ai,nqi])
                                            #loss2_2 = beta*(S[pi,nqi] + S[ai,si]) + 2*alpha*(T[pi,nqi] + T[ai,si])
                                            #gain2_2 = beta*(S[pi,si] + S[ai,nqi]) + 2*alpha*(T[pi,si] + T[ai,nqi])
                                            if gain3_1 > loss3:
                                                Swap[p,nq].append([(p,nq),(i,j),(a,s),(p,s),(i,nq),(a,j),gain3_1 - loss3])
                                            #if gain3_2 > loss3:
                                            #    Swap[p,nq].append([(p,nq),(i,j),(a,s),(p,j),(i,s),(a,nq),gain3_2 - loss3])
                                            #if gain2_2 > loss2_2:
                                            #    Swap[p,nq].append([(p,nq),(a,s),0,(p,s),(a,nq),0,gain2_2 - loss2_2])
                    Swap[edge].sort(key = lambda z:z[6], reverse=True)
                else:
                    print("This should never happen!")
        bestSwaps = [Swap[z][0] for z in Swap if Swap[z]]
         
    return (w, M)   # no further improvement is possible!

def findObjective(G1, G2, Gs, M0):
    # computes the value of the objective function:
    # S = total weight of matching, T = # conserved edges
    (S,T)=(0,0)
    for i in M0:
        if i in G1:
            j = M0[i]
            try: Gs.edge[i][j]
            except KeyError:
                S+=0
            else:
                S+=Gs.edge[i][j]['weight']
            for u in G1.neighbors(i):
                if u in M0:
                    v = M0[u]
                    if G2.has_edge(j,v):
                        T+=1
    return (S,T)

def findObjective2(G1, G2, M0):
    # computes the value of the objective function:
    # S = total weight of matching, T = # conserved edges
    T = 0
    for i in M0:
        if i in G1:
            j = M0[i]
            #S+=Gs.edge[i][j]['weight']
            for u in G1.neighbors(i):
                if u in M0:
                    v = M0[u]
                    if G2.has_edge(j,v):
                        T+=1
    return T
    
def processOnce(graph1, graph2, graphS, M0, alpha, c):
    # wrapper for a single run of the main algorithm
    if graph1.number_of_edges() > graph2.number_of_edges():
        return mainAlgo(graph2, graph1, graphS,  M0, alpha, c, Type = float, override = False)
    else:
        return mainAlgo(graph1, graph2, graphS,  M0, alpha, c, Type = float, override = False)

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
