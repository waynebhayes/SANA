import sys
sys.path.append("..")
import graph

def coverage(yeast, human, subgraph):
    y = len(subgraph) / (yeast.num_edges() / 2) * 100.0
    h = len(subgraph) / (human.num_edges() / 2) * 100.0
    return (y,h)

def ec1score(g1, induced_subgraph):
    return len(induced_subgraph) / (g1.num_edges() / 2 ) * 100

def ec2score( g2, induced_subgraph):
    return len(induced_subgraph) / (g2.num_edges() / 2 ) * 100

def s3score(g1, g2, pairs, subgraph):
    print("calculating s3..")
    aligned_edges = len(subgraph)
    u2in = unaligned_edges_g2_in(g1, g2, pairs, subgraph)
    denom = aligned_edges + (g1.num_edges() / 2) + len(u2in) 
    return aligned_edges/denom

