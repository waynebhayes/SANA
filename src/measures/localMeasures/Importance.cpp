#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Importance.hpp"

using namespace std;

const uint Importance::d = 10;
const double Importance::lambda = 0.2;

Importance::Importance(Graph* G1, Graph* G2) : LocalMeasure(G1, G2, "importance") {
    string fileName = autogenMatricesFolder+G1->getName()+"_"+G2->getName()+"_importance.bin";
    loadBinSimMatrix(fileName);
}

Importance::~Importance() {
}

vector<vector<double> > Importance::initEdgeWeights(const Graph& G) {
    uint n = G.getNumNodes();
    vector<vector<double> > edgeWeights(n, vector<double> (n, 0));
    vector<vector<bool> > adjMatrix(n, vector<bool> (n));
    G.getAdjMatrix(adjMatrix);
    for (uint i = 0; i < n; i++) {
        for (uint j = 0; j < n; j++) {
            if (adjMatrix[i][j]) edgeWeights[i][j] = 1;
        }
    }
    return edgeWeights;
}

struct DegreeComp {
    DegreeComp(vector<vector<ushort> > const *adjLists) {
        this->adjLists = adjLists;
    }

    bool operator() (ushort i, ushort j) {
        return ((*adjLists)[i].size() < (*adjLists)[j].size());
    }

    vector<vector<ushort> > const *adjLists;
};

vector<ushort> Importance::getNodesSortedByDegree(const vector<vector<ushort> >& adjLists) {
    uint n = adjLists.size();
    vector<ushort> nodes(n);
    for (ushort i = 0; i < n; i++) {
        nodes[i] = i;
    }
    sort(nodes.begin(), nodes.end(), DegreeComp(&adjLists));
    vector<ushort> res(0);
    for (ushort i = 0; i < n; i++) {
        if (adjLists[nodes[i]].size() > d) return res;
        res.push_back(nodes[i]);
    }
    error("no nodes left");
    return res;//dummy return
}

void Importance::removeFromAdjList(vector<ushort>& list, ushort u) {
    for (uint i = 0; i < list.size(); i++) {
        if (list[i] == u) list.erase(list.begin()+i);
    }
}

void Importance::normalizeImportances(vector<double>& v) {
    double maxim = 0;
    for (double d : v) {
        if (d > maxim) maxim = d;
    }
    for (uint i = 0; i < v.size(); i++) {
        v[i] = v[i]/maxim;
    }
}

vector<double> Importance::getImportances(const Graph& G) {
    uint n = G.getNumNodes();

    vector<double> nodeWeights(n, 0);
    vector<vector<double> > edgeWeights = initEdgeWeights(G);

    //adjLists will change as we remove nodes from G
    vector<vector<ushort> > adjLists(n);
    G.getAdjLists(adjLists);

    //as opposed to adjLists, degrees remain true to the original graph
    vector<uint> degrees(n);
    for (uint i = 0; i < n; i++) degrees[i] = adjLists[i].size();

    vector<ushort> nodesSortedByDegree = getNodesSortedByDegree(adjLists); //returns only the nodes with degree <= d
    for (ushort u : nodesSortedByDegree) {
        //update neighbors' weights
        if (degrees[u] == 1) {
            for (ushort v : adjLists[u]) {
                nodeWeights[v] += nodeWeights[u] + edgeWeights[u][v];
            }
        }
        else {
            double uWeight = nodeWeights[u];
            for (ushort v : adjLists[u]) {
                uWeight += edgeWeights[u][v];
            }

            for (uint i = 0; i < adjLists[u].size(); i++) {
                ushort v1 = adjLists[u][i];
                for (uint j = i+1; j < adjLists[u].size(); j++) {
                    ushort v2 = adjLists[u][j];
                    double numNeighbors = adjLists[u].size();
                    edgeWeights[v1][v2] +=
                        uWeight/((numNeighbors*(numNeighbors-1))/2.0);
                }
            }
        }

        for (ushort v : adjLists[u]) {
            removeFromAdjList(adjLists[v], u);
        }

        nodeWeights[u] = 0;
        /* The paper says: "When one node is removed, its adjacent
        edges are also removed and the weight of the removed node and
        edges are allocated to their neighboring nodes and edges. In this
        way, the topological information is propagated from a node to
        its neighbors."
        It is not clear whether when a node is removed it loses its weight.
        The wording seems to indicate so, but then all the nodes with degree <=10 (d)
        would end up with weight 0, and many mappins would be random (this is no longer
        true when also adding sequnece similarity).
        I assume here that they don't keep their weight (to change this, comment the previous line)
        Note: in a test, the scores were similar in both cases */
        
        /* same story with edge weights... */
        for (uint i = 0; i < n; i++) {
            edgeWeights[u][i] = edgeWeights[i][u] = 0;
        }
    }

    //compute importances
    G.getAdjLists(adjLists); //restore original adjLists
    vector<double> res(n);
    for (uint u = 0; u < n; u++) {
        double edgeWeightSum = 0;
        for (ushort v : adjLists[u]) {
            edgeWeightSum += edgeWeights[u][v];
        }
        res[u] = nodeWeights[u] + edgeWeightSum;
    }

    normalizeImportances(res);
    return res;
}

void Importance::initSimMatrix() {
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));
    vector<double> scoresG1 = getImportances(*G1);
    vector<double> scoresG2 = getImportances(*G2);
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            sims[i][j] = min(scoresG1[i],scoresG2[j]);
        }
    }
}

bool Importance::hasNodesWithEnoughDegree(const Graph& G) {
    uint n = G.getNumNodes();
    vector<vector<ushort> > adjLists(n);
    G.getAdjLists(adjLists);
    vector<ushort> nodes(n);
    for (ushort i = 0; i < n; i++) {
        nodes[i] = i;
    }
    sort(nodes.begin(), nodes.end(), DegreeComp(&adjLists));
    return adjLists[nodes[n-1]].size() > d;
}

bool Importance::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return hasNodesWithEnoughDegree(*G1) and hasNodesWithEnoughDegree(*G2);
}
