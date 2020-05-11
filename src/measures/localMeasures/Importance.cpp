#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Importance.hpp"

using namespace std;

const uint Importance::d = 10;
const double Importance::lambda = 0.2; // best value according to the HubAlign paper.

Importance::Importance(const Graph* G1, const Graph* G2) : LocalMeasure(G1, G2, "importance") {
    string subfolder = autogenMatricesFolder+getName()+"/";
    createFolder(subfolder);
    string fileName = subfolder+G1->getName()+"_"+G2->getName()+"_importance.bin";
    loadBinSimMatrix(fileName);
}

Importance::~Importance() {
}

vector<vector<double>> Importance::initEdgeWeights(const Graph& G) {
    uint n = G.getNumNodes();
    vector<vector<double>> edgeWeights(n, vector<double> (n, 0));
#ifdef MULTI_PAIRWISE
    throw runtime_error("Importance not implemented for weighted Graphs");
#endif
    for (uint i = 0; i < n; i++) {
        for (uint j = 0; j < n; j++) {
            if (G.hasEdge(i,j)) edgeWeights[i][j] = 1;
        }
    }
    return edgeWeights;
}

struct DegreeComp {
    DegreeComp(vector<vector<uint>> const *adjLists) {
        this->adjLists = adjLists;
    }

    // Note: the paper provides a deterministic algorithm that, unfortunately, means that nodes that "in spirit"
    // should have the same importance---meaning they have the same degree at some point---will end up with
    // different importances due to one being deleted before the other.  This is bad because if you simply
    // reorder the nodes in the input file, you can get a *wildly* different answer.  We probably need to
    // ameliorate this in some way.  Ideas: easiest is probably to intentionally introduce randomness, compute
    // "importance" many times based on random shuffles of the node/edge orderings.  Probably a better idea is
    // to figure out some way to delete all equally-valued importance nodes simultaneously rather than imposing
    // an arbitrary order.
    bool operator() (uint i, uint j) {
        int size1 = (*adjLists)[i].size(), size2 = (*adjLists)[j].size();
        if (size1 == size2)
        return 0;
    else
        return (size1 < size2);
    }

    vector<vector<uint>> const *adjLists;
};

vector<uint> Importance::getNodesSortedByDegree(const vector<vector<uint>>& adjLists) {
    uint n = adjLists.size();
    vector<uint> nodes(n);
    for (uint i = 0; i < n; i++) {
        nodes[i] = i;
    }
    sort(nodes.begin(), nodes.end(), DegreeComp(&adjLists));
    vector<uint> res(0);
    for (uint i = 0; i < n; i++) {
        if (adjLists[nodes[i]].size() > d) return res;
        res.push_back(nodes[i]);
    }
    throw runtime_error("no nodes left");
    return res;//dummy return
}

void Importance::removeFromAdjList(vector<uint>& list, uint u) {
    for (uint i = 0; i < list.size(); i++) {
        if (list[i] == u) list.erase(list.begin()+i);
    }
}

void Importance::normalizeImportances(vector<double>& v) {
    double maxim = 0;
    for (double val : v) {
    assert(val >= 0);
        if (val > maxim) maxim = val;
    }
    for (uint i = 0; i < v.size(); i++) {
        v[i] = v[i]/maxim;
    }
}

vector<double> Importance::getImportances(const Graph& G) {
    uint n = G.getNumNodes();

    vector<double> nodeWeights(n, 0);
    vector<vector<double>> edgeWeights = initEdgeWeights(G);

    //adjLists will change as we remove nodes from G
    vector<vector<uint>> adjLists(G.getAdjLists()->begin(), G.getAdjLists()->end());

    //as opposed to adjLists, degrees remain true to the original graph
    vector<uint> degrees(n);
    for (uint i = 0; i < n; i++) degrees[i] = adjLists[i].size();

    vector<uint> nodesSortedByDegree = getNodesSortedByDegree(adjLists); //returns only the nodes with degree <= d
    for (uint u : nodesSortedByDegree) {
        //update neighbors' weights
        if (degrees[u] == 1) {
            for (uint v : adjLists[u]) {
                nodeWeights[v] += nodeWeights[u] + edgeWeights[u][v];
            }
        }
        else {
            double uWeight = nodeWeights[u];
            for (uint v : adjLists[u]) {
                uWeight += edgeWeights[u][v];
            }

            for (uint i = 0; i < adjLists[u].size(); i++) {
                uint v1 = adjLists[u][i];
                for (uint j = i+1; j < adjLists[u].size(); j++) {
                    uint v2 = adjLists[u][j];
                    double numNeighbors = adjLists[u].size();
                    edgeWeights[v1][v2] +=
                        uWeight/((numNeighbors*(numNeighbors-1))/2.0);
                }
            }
        }

        for (uint v : adjLists[u]) {
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
        would end up with weight 0, and many mappings would be random (this is no longer
        true when also adding sequnece similarity).
        I assume here that they don't keep their weight (to change this, comment the previous line)
        Note: in a test, the scores were similar in both cases */

        /* same story with edge weights... */
        for (uint i = 0; i < n; i++) {
            edgeWeights[u][i] = edgeWeights[i][u] = 0;
        }
    }

    //compute importances
    const vector<vector<uint>> * originalAdjLists = G.getAdjLists();
    vector<double> res(n);
    for (uint u = 0; u < n; u++) {
        double edgeWeightSum = 0;
        for (uint v : (*originalAdjLists)[u]) {
            edgeWeightSum += edgeWeights[u][v];
        }
        res[u] = nodeWeights[u] + edgeWeightSum;
    }
    normalizeImportances(res);
    return res;
}

void Importance::initSimMatrix() {
    uint n1 = G1->getNumNodes(), n2 = G2->getNumNodes();
    sims = vector<vector<float>> (n1, vector<float> (n2, 0));

    const uint NUM_SHUFFLES = 30;
    cout << "Creating average importances from " << NUM_SHUFFLES << " shuffles of the nodes of G1 and G2\n";
    for(uint shuf = 0 ; shuf < NUM_SHUFFLES; shuf++) {
        vector<uint> H1ToG1Map, H2ToG2Map;
        Graph H1 = G1->shuffledGraph(H1ToG1Map);
        Graph H2 = G2->shuffledGraph(H2ToG2Map);
        for(uint i=0; i<n1; i++) assert(G1->getNumNbrs(H1ToG1Map[i]) == H1.getNumNbrs(i));
        for(uint i=0; i<n2; i++) assert(G2->getNumNbrs(H2ToG2Map[i]) == H2.getNumNbrs(i));
        vector<double> scoresH1 = getImportances(H1);
        vector<double> scoresH2 = getImportances(H2);
        for (uint i = 0; i < n1; i++)
            for (uint j = 0; j < n2; j++)
              sims[H1ToG1Map[i]][H2ToG2Map[j]] += min(scoresH1[i],scoresH2[j]);
    }
    for (uint i = 0; i < n1; i++)
        for (uint j = 0; j < n2; j++)
            sims[i][j] /= NUM_SHUFFLES;
}

bool Importance::hasNodesWithEnoughDegree(const Graph& G) {
    uint n = G.getNumNodes();
    vector<uint> nodes(n);
    for (uint i = 0; i < n; i++) {
        nodes[i] = i;
    }
    sort(nodes.begin(), nodes.end(), DegreeComp(G.getAdjLists()));
    return (*G.getAdjLists())[nodes[n-1]].size() > d;
}

bool Importance::fulfillsPrereqs(const Graph* G1, const Graph* G2) {
    return hasNodesWithEnoughDegree(*G1) and hasNodesWithEnoughDegree(*G2);
}
