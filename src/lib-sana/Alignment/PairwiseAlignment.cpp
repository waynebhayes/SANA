#include "PairwiseAlignment.hpp"
#include <algorithm>

using namespace std;
//Default Constructor and Destructor Already Defined
PairwiseAlignment::PairwiseAlignment(const PairwiseAlignment &rhs) : A(rhs.A) {}

bool PairwiseAlignment::printDefinitionErrors(const Graph &G1, const Graph &G2) {
    uint n1 = G1.GetNumNodes();
    uint n2 = G2.GetNumNodes();
    unordered_map<ushort,string> G1Names = G1.getIndexToNodeNameMap();
    unordered_map<ushort,string> G2Names = G2.getIndexToNodeNameMap();

    vector<bool> G2AssignedNodes(n2, false);
    int count = 0;
    if (A.size() != n1) cout<<"Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cout <<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cout <<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<" ("<<G2Names[A[i]]<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
    }
    if(count) return true; //We had some count of errors
    return false;
}

unsigned int PairwiseAlignment::numAlignedEdges(const Graph &G1, const Graph &G2) const {
    vector < vector<unsigned int > > G1EdgeList = G1.getAdjList();
    vector < vector<unsigned int > > G2EdgeList = G2.getAdjList();

    unsigned int totalAligned = 0;
    unsigned int currRowNode = 0;
    for(const auto& row : G1EdgeList) { //This selects the row vector, starts at node "0"
        ushort rowNode = currRowNode;
        for(const auto &col : row) { //This select the column vector, int stored within each column is the destination node
            //Curr RowNode and this destination (col) node make an edge
            ushort edge1_R = rowNode;
            ushort edge1_C = col;
            //Check to see if graph2 has the same edge, we have to rely on linear search here because we don't have an adj
            //matrix which would allow us to do 2d array access, the edges are unordered in an adjList
            if(find(G2EdgeList[edge1_R].begin(),G2EdgeList[edge1_R].end(),edge1_C) != G2EdgeList[edge1_R].end()) {
                //The edges align
                ++totalAligned;
            }
        }
        ++currRowNode; //Iterate our rowNode
    }
    return totalAligned;
}

typedef unordered_map<ushort,string> NodeIndexMap;
void PairwiseAlignment::dumpEdgeList(const Graph &G1, const Graph &G2, ostream &edgeListStream) const {
    NodeIndexMap mapG1 = G1.getIndexToNodeNameMap();
    NodeIndexMap mapG2 = G2.getIndexToNodeNameMap();
    for(uint i = 0; i < size(); ++i)
        edgeListStream << mapG1[i] << "\t" << mapG2[A[i]] << endl;
}

vector<ushort> PairwiseAlignment::getMapping() const {
    return A;
}

