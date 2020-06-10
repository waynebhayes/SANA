#include "MultiS3.hpp"
#include <cmath>

unsigned NUM_GRAPHS;
unsigned MultiS3::denom = 1;
unsigned MultiS3::numer = 1;
double MultiS3::_type = 0;
vector<uint> MultiS3::shadowDegree;

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
    
    //cout << "EEEEEEEEEEEEEEEEnter MultiS3......" << endl;
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
      _type=1;//default
//    _type = type;
//    if (type==1){
//        cout<<"Multi S3: denom = ee"<<endl;
//    }else if(type==0){
//        cout<<"Multi S3: denom = default"<<endl;
//    }    
    numerator_type   = _numerator_type  ;
    denominator_type = _denominator_type;
    //cout<<"Multi S3: numer = ";
    switch(numerator_type){
        case 1:
            cout<<"ra_i"<<endl;
            break; 
        case 2:
            cout<<"la_i"<<endl;
            break;
        case 3:
            cout<<"la_global"<<endl;
            break;
        case 4:
            cout<<"ra_global"<<endl;
            break;
        default:
            cout<<"default"<<endl;
    }
    cout<<"Multi S3: denom = ";
    switch(denominator_type){
        case 1:
            cout<<"rt_i"<<endl;
            break;
        case 2:
            cout<<"ee_i"<<endl;
            break;
        case 3:
            cout<<"ee_global"<<endl;
            break;
        case 4:
            cout<<"rt_global"<<endl;
            break;
        default:
            cout<<"default"<<endl;
    }

    //cout << "Multi S3: NUM_GRAPHS = " << NUM_GRAPHS << endl;
    degreesInit = false;
    //G1->printStats(0, cout);
    //G2->printStats(0, cout);
#endif
}

MultiS3::~MultiS3() {
}

// I think this is correct, but it's mis-used in computeDenom
void MultiS3::initDegrees(const Alignment& A, const Graph& G1, const Graph& G2)
{
    shadowDegree = vector<uint>(G2.getNumNodes(), 0);
    Matrix<MATRIX_UNIT> G1Matrix;
    G1.getMatrix(G1Matrix);
    
    Matrix<MATRIX_UNIT> G2Matrix;
    G2.getMatrix(G2Matrix);
    
    const uint n2 = G2.getNumNodes();
    for (uint i = 0; i < n2; ++i) // for each shadow node i
    {
        for (uint j = 0; j < n2; ++j) // and for all possible neighbors of node i
        {
	    if(i==j) assert(G2Matrix[i][j]==0); // no self loops
            shadowDegree[i] += G2Matrix[i][j]; // increment the "total degree" of node i by the weight of edge(i,j)
        }
    }

    // Now add in the edges from G1
    const uint n1 = G1.getNumNodes();
    for (uint i = 0; i < n1; ++i)
    {
        shadowDegree[A[i]] += 1;
    }
    
    degreesInit = true;
}

unsigned MultiS3::computeDenom(const Alignment& A, const Graph& G1, const Graph& G2)
{
    LaddersUnderG1 = 0;
#if 1    
    const uint n1 = G1.getNumNodes();
    for (uint i = 0; i < n1 - 1; ++i)
    {
        for (uint j = i + 1; j < n1; ++j)
        {
            if (shadowDegree[i] > 0) 
            {// this makes no sense: shadowDegree index is a G2 node, not G1, and even if it was A[i] this doesn't imply a ladder
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
        if (shadowDegree[hole] > 0)
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
    //cout << "Incdenom: " << denom << endl;
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
    //if(newNumer != numer) cerr << "inc eval MS3numer wrong: should be "<<newNumer<<" but is "<<numer << '\n';
    //cout << "numer: " << numer << "newNumer: " << newNumer << endl;
    numer = newNumer;
    //cout << "Evaldenom: " << denom << endl;
    if (   (   (numerator_type==ra_i   or numerator_type==ra_global)
           and (denominator_type==ee_i or denominator_type==ee_global))
        or (numerator_type==_default and denominator_type==_default)  ){
        //which measures need to be divided by the number of networks: 
        //    whenever the numerator is RA* and denominator is EE* (any EE).
        // or using default ms3
        return double(newNumer) / denom / NUM_GRAPHS;
    }else{
        return double(newNumer) / denom / NUM_GRAPHS;
    }
#else
    return 0.0;
#endif
}
