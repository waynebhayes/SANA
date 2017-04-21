#!/usr/bin/env python3
import argparse
import pathlib
import sys

class Graph:

    def __init__(self, file_name:str, is_GW:bool=True) -> None:
        self.node_map,self.edge_list = self.read_GW(file_name) if is_GW else self.read_edge_list(file_name)

    def read_GW(self, file_name:str) -> (dict,[[int]]):
        with open(file_name,mode='r') as f:
            for i in range(4):
                next(f)
            count = int(f.readline().strip())
            edge_list = [[] for x in range(count)]
            node_map = dict()

            for i in range(count):
                node_map[f.readline().strip()[2:-2]] = i

            edge_count = int(f.readline().strip())
            for i in range(edge_count):
                a = f.readline().split()
                node_a = int(a[0]) - 1
                node_b = int(a[1]) - 1
                edge_list[node_a].append(node_b)
                edge_list[node_b].append(node_a)
        return node_map,edge_list

    def read_edge_list(self, file_name:str) -> (dict,[[int]]):
        node_map = dict()
        node_num = 0
        edge_list = []
        with open(file_name,mode='r') as f:
            for line in f:
                node_a, node_b = line.strip().split()
                if node_a not in node_map:
                    node_map[node_a] = node_num
                    edge_list.append([])
                    node_num += 1
                else:
                    node_map[node_b] = node_num
                    edge_list.append([])
                    node_num += 1
                edge_list[node_map[node_a]].append(node_map[node_b])
                edge_list[node_map[node_b]].append(node_map[node_a])
        return node_map,edge_list

def read_alignment(file_name:str) -> [(str,str),]:
    with open(str(file_name),mode='r') as f:
        alignment = [tuple(line.strip().split()) for line in f]
    return alignment

def convert_alignment(alignment:[(str,str)], m_graph) -> [int]:
    int_alignment = [0 for x in range(len(alignment))]
    for node_name,s_node in alignment:
        node_num = m_graph.node_map[node_name]
        int_alignment[node_num] = int(s_node)
    return int_alignment

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create shadow network')
    parser.add_argument('-n','--networks', required=True, nargs='+')
    parser.add_argument('-a','--alignments', required=True, nargs='+')
    parser.add_argument('-s','--shadow-nodes', required=True, type=int)
    args = parser.parse_args()
    print(args.networks)
    print(args.alignments)

    s_el = [[] for x in range(args.shadow_nodes)]
    s_am = [[0 for x in range(args.shadow_nodes)] for y in range(args.shadow_nodes)]

    alignments = {pathlib.Path(f).stem:f for f in args.alignments}

    for network in args.networks:
        network_path = pathlib.Path(network)
        m_graph = Graph(network, network_path.suffix == '.gw')

        a = '{}-shadow'.format(network_path.stem)
        if a not in alignments:
            print('{}.align is not present in --alignments'.format(a), file=sys.stderr)

        alignment = read_alignment(alignments[a])
        int_alignment = convert_alignment(alignment, m_graph)

        for peg,hole in enumerate(int_alignment):
            for end_peg in m_graph.edge_list[peg]:
                end_hole = int_alignment[end_peg]
                if s_am[hole][end_hole] == 0:
                    s_el[hole].append(end_hole)
                s_am[hole][end_hole] += 1

    print('LEDA.GRAPH')
    print('string')
    print('short')
    print('-1')
    print(len(s_el))
    for node_a in range(len(s_el)):
        print('|{{shadow{}}}|'.format(node_a))
    print(int(sum(len(x) for x in s_el) / 2))
    for node_a in range(len(s_am)):
        for node_b in range(node_a, len(s_am)):
            if (s_am[node_a][node_b] > 0):
                print('{} {} 0 |{{{}}}|'.format(node_a+1, node_b+1, s_am[node_a][node_b]))

