#include "MultiS3.hpp"
#include <cmath>

unsigned MultiS3::denom = 0;
vector<uint> MultiS3::shadowDegrees;

MultiS3::MultiS3(Graph* G1, Graph* G2) : Measure(G1, G2, "ms3")
{
    //G1->printStats(0, cout);
    //G2->printStats(0, cout);
    initDegrees(*G2);
}

MultiS3::~MultiS3() {
}

void MultiS3::initDegrees(const Graph& G2)
{
    shadowDegrees = vector<uint>(G2.getNumNodes(), 0);
    
    vector<vector<uint>> G2AdjLists;
    G2.getAdjLists(G2AdjLists);
    
    Matrix<MATRIX_UNIT> G2Matrix;
    G2.getMatrix(G2Matrix);
    
    const uint n = G2AdjLists.size();
    uint neighbor;
    
    for (uint i = 0; i < n; ++i)
    {
        for (uint j = 0; j < G2AdjLists[i].size(); ++j)
        {
            neighbor = G2AdjLists[i][j];
            shadowDegrees[i] += G2Matrix[i][neighbor]; // +1 if G1 has edge here, since it was pruned?
        }
    }
}

unsigned MultiS3::getDenom(const Alignment& A, const Graph& G1, const Graph& G2) // not used for inc. eval so it's ok to keep ladder computation in here as it's O(n) 
{
    vector<vector<uint>> G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    uint node1, node2;
    LaddersUnderG1 = 0;
    unordered_set<uint> holes; // no duplicates allowed in cpp sets
    
    // Looping through edges is O(n1^2). Why can't this just be for(i=0;i<n1;i++) holes.insert(A[i]); ?
    for (const auto& edge : G1EdgeList)
    {
        node1 = edge[0], node2 = edge[1];
        holes.insert(A[node1]);
        holes.insert(A[node2]);
    }
    
    // I don't think this correctly handles the fact that G1 was pruned.
    for (const auto& hole : holes)
    {
        if (shadowDegrees[hole] > 0)
        {
            ++LaddersUnderG1;
        }
    }
    LaddersUnderG1 /= 2; // William sez: I don't think dividing by 2 is correct but that's ok
    // WH says: I think it does need division by 2...
    denom = LaddersUnderG1;
    return denom;
}

double MultiS3::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    getDenom(A, *G1, *G2);
    return double (A.multiS3Numerator(*G1, *G2)) / LaddersUnderG1;
#else
    return 0.0;
#endif
}
