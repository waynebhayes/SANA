from ../ import graph

def coverage(yeast, human, subgraph):
    y = len(subgraph) / (yeast.num_edges() / 2) * 100.0
    h = len(subgraph) / (human.num_edges() / 2) * 100.0
    return (y,h)

def ec1score(g1, induced_subgraph):
    return len(induced_subgraph) / (g1.num_edges() / 2 ) * 100

def ec2score( g2, induced_subgraph):
    return len(induced_subgraph) / (g2.num_edges() / 2 ) * 100

