#include "PairwiseAlignment.hpp"
#include "Graph.hpp"
#include "Utils.hpp"

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