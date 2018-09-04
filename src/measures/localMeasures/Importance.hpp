#ifndef IMPORTANCE_HPP
#define IMPORTANCE_HPP
#include "LocalMeasure.hpp"

class Importance: public LocalMeasure {
public:
    Importance(Graph* G1, Graph* G2);
    virtual ~Importance();

    static bool fulfillsPrereqs(Graph* G1, Graph* G2);
    
private:
    /* we use a minimum-degree heuristics algorithm to calcu-
    late the topological importance of nodes and edges, starting from
    the nodes with degree one and stopping at those with degree d */
    static const uint d;
    /* lambda controls the importance of
    the edge weight relative to the node weight. Empirically lambda=0.2
    yields a biologically more meaningful alignment */
    static const double lambda;

    void initSimMatrix();

    static vector<double> getImportances(const Graph& G);

    static vector<vector<double> > initEdgeWeights(const Graph& G);
    static vector<uint> getNodesSortedByDegree(const vector<vector<uint> >& adjLists);
    static void removeFromAdjList(vector<uint>& list, uint u);
    static void normalizeImportances(vector<double>& v);

    static bool hasNodesWithEnoughDegree(const Graph& G);
};

#endif
