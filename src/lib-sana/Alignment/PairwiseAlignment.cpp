#include "PairwiseAlignment.hpp"
#include "Utils.hpp"

using namespace std;
//Default Constructor and Destructor Already Defined
PairwiseAlignment::PairwiseAlignment(const PairwiseAlignment &rhs) : A(rhs.A) {}

PairwiseAlignment::PairwiseAlignment(Graph *G1, Graph *G2, const vector<vector<string> > &mapList) {
    map<string,ushort> mapG1 = G1->getNodeNameToIndexMap();
    map<string,ushort> mapG2 = G2->getNodeNameToIndexMap();
    ushort n1 = mapList.size();
    ushort n2 = G2->GetNumNodes();
    A = vector<ushort>(G1->GetNumNodes(),n2);
    for(size_t i = 0; i < n1; ++i) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];
        A[mapG1[nodeG1]] = mapG2[nodeG2];
    }
}

void PairwiseAlignment::printDefinitionErrors(const Graph &G1, const Graph &G2) {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    unordered_map<ushort,string> G1Names = G1.getIndexToNodeNameMap();
    unordered_map<ushort,string> G2Names = G2.getIndexToNodeNameMap();

    vector<bool> G2AssignedNodes(n2, false);
    int count = 0;
    if (A.size() != n1) cout<<"Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cout<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cout<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<" ("<<G2Names[A[i]]<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
    }
}