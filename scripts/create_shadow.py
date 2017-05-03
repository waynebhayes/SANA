#!/usr/bin/env python3
import argparse
import pathlib
import sys
import os

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
    int_alignment = [None for x in range(len(alignment))]
    for s_node,node_name in alignment:
        try:
            node_num = m_graph.node_map[node_name]
        except KeyError:
            continue
        int_alignment[node_num] = int(s_node[6:])
    return int_alignment

def read_int_alignment(file_name:str) -> [int]:
    with open(str(file_name),mode='r') as f:
        alignment = [int(x) for x in f.readline().strip().split()]
    return alignment


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create shadow network')
    parser.add_argument('-n','--networks', required=True, nargs='+')
    parser.add_argument('-a','--alignments', required=True, nargs='+')
    parser.add_argument('-s','--shadow-nodes', required=True, type=int)
    parser.add_argument('-v','--verbose', action='store_true')
    parser.add_argument('-c','--compact', action='store_true')
    args = parser.parse_args()
    if args.verbose:
        print(args.networks,file=sys.stderr)
        print(args.alignments, file=sys.stderr)

    s_el = [[] for x in range(args.shadow_nodes)]
    s_am = [[0 for x in range(args.shadow_nodes)] for y in range(args.shadow_nodes)]

    alignments = {pathlib.Path(f).stem:f for f in args.alignments}
    if args.compact:
        def m_compact(x,y):
            return read_int_alignment(x)
        m_function = m_compact
    else:
        def m_compact(x,y):
            m_alignment = convert_alignment(read_alignment(x),y)
            m_path,m_ext = os.path.splitext(x)
            with open(m_path+'.out',mode='w') as f:
                print(' '.join(str(x) for x in m_alignment), file=f)
            return m_alignment
        m_function = m_compact

    for network in args.networks:
        network_path = pathlib.Path(network)
        m_graph = Graph(network, network_path.suffix == '.gw')

        a = 'shadow-{}'.format(network_path.stem)
        if a not in alignments:
            print('{} not listed in --alignments'.format(a), 
                    file=sys.stderr)
            print(args.alignments, file=sys.stderr)
            sys.exit(1)
        int_alignment = m_function(alignments[a],m_graph)

        for peg,hole in enumerate(int_alignment):
            if hole == None:
                continue
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

