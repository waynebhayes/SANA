#include "MultiS3.hpp"
#include <cmath>

unsigned NUM_GRAPHS;
unsigned MultiS3::denom = 0;
vector<uint> MultiS3::totalDegrees;

MultiS3::MultiS3(Graph* G1, Graph* G2) : Measure(G1, G2, "ms3")
{
    extern char *getetv(char*);
    char *s = getenv((char*)"NUM_GRAPHS");
    if (s)
    {
        assert(1 == sscanf(s, "%u",&NUM_GRAPHS));
    }
    else
    {
        cout << "Warning: NUM_GRAPHS should be an environment variable; setting to 2 for now\n";
        NUM_GRAPHS = 2;
    }
    cout << "Multi S3: NUM_GRAPHS = " << NUM_GRAPHS << endl;
    degreesInit = false;
    //G1->printStats(0, cout);
    //G2->printStats(0, cout);
}

MultiS3::~MultiS3() {
}

void MultiS3::initDegrees(const Alignment& A, const Graph& G1, const Graph& G2)
{
    totalDegrees = vector<uint>(G2.getNumNodes(), 0);
#if 0
    vector<vector<uint> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    
    vector<vector<uint>> G2AdjLists;
    G2.getAdjLists(G2AdjLists);
    
    uint node1, node2;

    for (const auto& edge: G1EdgeList)
    {
        node1 = edge[0], node2 = edge[1];
        G2Matrix[A[node1]][A[node2]] += 1; // +1 because G1 was pruned out of G2
    }
#endif    
    Matrix<MATRIX_UNIT> G2Matrix;
    G2.getMatrix(G2Matrix);
    
    const uint n1 = G1.getNumNodes();
    for (uint i = 0; i < n1; ++i)
    {
        totalDegrees[A[i]] += 1;
    }
    
    const uint n2 = G2.getNumNodes();
    for (uint i = 0; i < n2; ++i)
    {
        for (uint j = 0; j < n2; ++j)
        {
            totalDegrees[i] += G2Matrix[i][j];
        }
    }
    degreesInit = true;
}

unsigned MultiS3::computeDenom(const Alignment& A, const Graph& G1, const Graph& G2)
{
    LaddersUnderG1 = 0;
#if 1    
    const uint n = G1.getNumNodes();
    for (uint i = 0; i < n - 1; ++i)
    {
        for (uint j = i + 1; j < n; ++j)
        {
            if (totalDegrees[i] > 0)
            {
                ++LaddersUnderG1;
            }
        }
    }
#else
    vector<vector<uint>> G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    uint node1, node2;
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
        if (totalDegrees[hole] > 0)
        {
            ++LaddersUnderG1;
        }
    }
#endif
    assert(LaddersUnderG1 % 2 == 0);
    LaddersUnderG1 /= 2;
    denom = LaddersUnderG1;
    return denom;
}

double MultiS3::eval(const Alignment& A)
{
#if MULTI_PAIRWISE
    if (!degreesInit)
    {
        initDegrees(A, *G1, *G2);
    }
    computeDenom(A, *G1, *G2);
    return double ((A.multiS3Numerator(*G1, *G2)) / LaddersUnderG1) / NUM_GRAPHS;
#else
    return 0.0;
#endif
}
