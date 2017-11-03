/*
 * compute_local_subgraphs.cpp
 *
 *  Created on: Oct 21, 2008
 *      Author: weng
 */

#include "../common.h"
#include <utility>
using std::pair;
#include <boost/graph/adjacency_list.hpp>
using boost::adjacency_list;
using boost::vecS;
using boost::undirectedS;
#include <boost/graph/graph_utility.hpp>
using boost::vertex_index;
//#include <boost/graph/graph_traits.hpp>

void bgl_notes() {
    // create a typedef for the Graph type
    typedef adjacency_list<vecS, vecS, undirectedS> Graph;

    // Make convenient labels for the vertices
    enum { A, B, C, D, E, N };
    const int num_vertices = N;

    typedef std::pair<int, int> Edge;
    Edge edge_array[] = { Edge(A,B), Edge(A,D), Edge(C,A), Edge(D,C),
            Edge(C,E), Edge(B,D), Edge(D,E) };
    const int num_edges = sizeof(edge_array)/sizeof(edge_array[0]);

    // declare a graph object
    Graph g(num_vertices);

    // add the edges to the graph object
    for (int i = 0; i < num_edges; ++i)
      add_edge(edge_array[i].first, edge_array[i].second, g);

    print_graph(g, get(vertex_index, g));
}
