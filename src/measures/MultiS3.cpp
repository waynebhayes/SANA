#include "MultiS3.hpp"
#include <cmath>

unsigned NUM_GRAPHS;
unsigned MultiS3::denom = 1;
unsigned MultiS3::numer = 1;
double MultiS3::_type = 0;
vector<uint> MultiS3::totalDegrees;

unsigned MultiS3::numerator_type   = 0; // 0 for default version of ms3
unsigned MultiS3::denominator_type = 0;
unsigned MultiS3::_default         = 0; 
//numerator type
unsigned MultiS3::ra_i             = 1;
unsigned MultiS3::la_i             = 2;
unsigned MultiS3::la_global        = 3;
unsigned MultiS3::ra_global        = 4;
//denominator type
unsigned MultiS3::rt_i             = 1;
unsigned MultiS3::ee_i             = 2;
unsigned MultiS3::ee_global        = 3;
unsigned MultiS3::rt_global        = 4;


MultiS3::MultiS3(Graph* G1, Graph* G2, int _numerator_type, int _denominator_type) : Measure(G1, G2, "ms3")
{
#if MULTI_PAIRWISE
    
    extern char *getetv(char*);
    char *s = getenv((char*)"NUM_GRAPHS");
    if (s)
    {
        assert(1 == sscanf(s, "%u",&NUM_GRAPHS));
    }
    else
    {
        cerr << "Warning: NUM_GRAPHS should be an environment variable; setting to 2 for now\n";
        NUM_GRAPHS = 2;
    }
//    _type=0;//default
//    _type = type;
//    if (type==1){
//        cout<<"Multi S3: denom = ee"<<endl;
//    }else if(type==0){
//        cout<<"Multi S3: denom = default"<<endl;
//    }    
    numerator_type   = _numerator_type  ;
    denominator_type = _denominator_type;
    cout<<"Multi S3: numer = "
    switch(numerator_type){
        case ra_i:
            cout<<"ra_i"<<endl;
            break; 
        case la_i:
            cout<<"la_i"<<endl;
            break;
        case la_global:
            cout<<"la_global"<<endl;
            break;
        case ra_global:
            cout<<"ra_global"<<endl;
            break;
        finally:
            cout<<"default"<<endl;
    }
    cout<<"Multi S3: denom = ";
    switch(denominator_type){
        case rt_i:
            cout<<"rt_i"<<endl;
            break;
        case ee_i:
            cout<<"ee_i"<<endl;
            break;
        case ee_global:
            cout<<"ee_global"<<endl;
            break;
        case rt_global:
            cout<<"rt_global"<<endl;
            break;
        finally:
            cout<<"default"<<endl;
    }

    cout << "Multi S3: NUM_GRAPHS = " << NUM_GRAPHS << endl;
    degreesInit = false;
    //G1->printStats(0, cout);
    //G2->printStats(0, cout);
#endif
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
    Matrix<MATRIX_UNIT> G1Matrix;
    G1.getMatrix(G1Matrix);
    
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
    //assert(LaddersUnderG1 % 2 == 0);
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
//    if (_type==1){
//        uint ne = A.numExposedEdges(*G1, *G2);
//        denom = ne;
//    }
//    else if (_type==0){
//        computeDenom(A, *G1, *G2);
//    }
    if (numerator_type==_default){
        computeDenom(A, *G1, *G2);
    }else{
        uint ne = A.multiS3Denominator(*G1, *G2);
        denom = ne;
    }
    unsigned newNumer = A.multiS3Numerator(*G1, *G2);
    if(newNumer != numer) cerr << "inc eval MS3numer wrong: should be "<<newNumer<<" but is "<<numer << '\n';
    numer = newNumer;
    if (   (   (numerator_type==ra_i   or numerator_type==ra_global)
           and (denominator_type==ee_i or denominator_type==ee_global))
        or (numerator_type==_default and denominator_type==_default)  ){
        //which measures need to be divided by the number of networks: 
        //    whenever the numerator is RA* and denominator is EE* (any EE).
        // or using default ms3
        return double(newNumer) / denom / NUM_GRAPHS;
    }else{
        return double(newNumer) / denom ;
    }
#else
    return 0.0;
#endif
}
