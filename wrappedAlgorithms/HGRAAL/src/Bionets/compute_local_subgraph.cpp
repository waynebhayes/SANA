/*
 * compute_local_subgraphs.cpp
 *
 *  Created on: Oct 21, 2008
 *      Author: weng
 */

#include "../common.h"

// STL
#include <utility>
using std::pair;
#include <set>
using std::set;

// BGL
#include <boost/graph/adjacency_list.hpp>
using boost::adjacency_list;
using boost::vecS;
using boost::undirectedS;
#include <boost/graph/graph_utility.hpp>
using boost::vertex_index;
#include <boost/graph/graph_traits.hpp>
using boost::graph_traits;
#include <boost/graph/properties.hpp>
using boost::property_map;
using boost::vertex_index_t;

typedef adjacency_list<vecS, vecS, undirectedS> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef property_map<Graph, vertex_index_t>::type IndexMap;

set<Vertex> * get_adjacent_vertex_set(Graph g, Vertex v) {
    typedef graph_traits<Graph>::adjacency_iterator AdjIt;
    typedef pair<AdjIt, AdjIt> AdjItPair;

    IndexMap index = get(vertex_index, g);
    set<Vertex> * neighbors = new set<Vertex>();
    for (AdjItPair av = adjacent_vertices(v, g);
            av.first != av.second; ++av.first)
        neighbors->insert(*(av.first));
    return neighbors;
}

set<Vertex> * compute_local_subgraph(Graph g, Vertex v) {
    set<Vertex> * interior_vertices = new set<Vertex>();
    set<Vertex> frontier_vertices;
    interior_vertices->insert(v);
    frontier_vertices.insert(v);
    int depth = 2;
    for (int i = 0; i < depth; ++i) {
        set<Vertex> frontier_neighbors;
        for (set<Vertex>::iterator it = frontier_vertices.begin();
                it != frontier_vertices.end(); ++it) {
            set<Vertex> * neighbors = get_adjacent_vertex_set(g, *it);
            for (set<Vertex>::iterator jt = neighbors->begin();
                    jt != neighbors->end(); ++jt)
                frontier_neighbors.insert(*jt);
        }
        frontier_vertices.clear();
        for (set<Vertex>::iterator it = frontier_neighbors.begin();
                it != frontier_neighbors.end(); ++it)
            if (interior_vertices->count(*it) == 0) {
                frontier_vertices.insert(*it);
                interior_vertices->insert(*it);
            }
    }
    return interior_vertices;
}

void run_compute_local_subgraph() {
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

    set<Vertex> * interior_vertices = compute_local_subgraph(g, A);

    IndexMap index = get(vertex_index, g);
    for (set<Vertex>::iterator it = interior_vertices->begin();
            it != interior_vertices->end(); ++it)
        cout << index[*it] << " ";
    cout << endl;

    print_graph(g, get(vertex_index, g));
}
