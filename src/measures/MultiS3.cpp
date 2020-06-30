#include "MultiS3.hpp"
#include "EdgeExposure.hpp"
#include <cmath>

uint NUM_GRAPHS;
uint MultiS3::denom = 1;
uint MultiS3::numer = 1;
double MultiS3::_type = 0;
bool MultiS3::degreesInit = false;
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

MultiS3::MultiS3(const Graph* G1, const Graph* G2, int _numerator_type, int _denominator_type) : Measure(G1, G2, "ms3") {
#ifdef MULTI_PAIRWISE
    extern char *getetv(char*);
    char *s = getenv((char*)"NUM_GRAPHS");
    if (s) {
        assert(1 == sscanf(s, "%u",&NUM_GRAPHS));
    }
    else {
        cerr << "Warning: NUM_GRAPHS should be an environment variable; setting to 2 for now\n";
        NUM_GRAPHS = 2;
    }
    //_type=0;//default
    numerator_type   = _numerator_type  ;
    denominator_type = _denominator_type;
    
    cout<<"MultiS3: numer_type = ";
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
    
    cout<<"MultiS3: denom_type = ";
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
    
    cout << "Multi S3: NUM_GRAPHS = " << NUM_GRAPHS << endl;
    degreesInit = false;
#endif
}

MultiS3::~MultiS3() {}

void MultiS3::setDenom(const Alignment& A) {
    denom = 0;
    const uint n1 = G1->getNumNodes();
    for (uint i = 0; i < n1; i++) {
        if (shadowDegree[i] > 0) denom += n1-1-i; // Nil correctly compressed this from SANA1.1 but I think SANA1.1 was wrong
    }
    denom /= 2;
}

uint MultiS3::computeNumer(const Alignment& A) const {
#ifdef MULTI_PAIRWISE
    uint ret = 0;
    uint node1, node2;

    switch (MultiS3::numerator_type){
        case 1:
        {
            for (const auto& edge: *(G1->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                if(MultiS3::numerator_type == MultiS3::ra_i){
                    // numerator is ra_i
                    if   (G2->getEdgeWeight(A[node1],A[node2]) >= 1){
                        ret += G2->getEdgeWeight(A[node1],A[node2]) + 1; // +1 because G1 was pruned out of G2
                    }
                }
            }
        }
            break;
        case 2:
        {
            for (const auto& edge: *(G1->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                if (MultiS3::numerator_type == MultiS3::la_i){
                    if (G2->getEdgeWeight(A[node1],A[node2]) >= 1){
                        ret += 1;               // +1 because it is a ladder
                    }
                }
            }
        }
            break;
        case 3:
        {
            const uint n1 = G1->getNumNodes();
            const uint n2 = G2->getNumNodes();
            vector<uint> reverse_A = vector<uint> (n2,n1);// value of n1 represents not used
            for(uint i=0; i< n1; i++){
                reverse_A[A[i]] = i;
            }
            for (const auto& edge: *(G2->getEdgeList())){
                node1 = edge[0], node2 = edge[1];
                if (MultiS3::numerator_type == MultiS3::la_global){
                    if ((G2->getEdgeWeight(node1,node2) > 1) or (reverse_A[node1] < n1 and reverse_A[node2] < n1 and G2->getEdgeWeight(node1,node2) + G1->getEdgeWeight(reverse_A[node1],reverse_A[node2]) > 1)){
                        ret += 1; // +1 because G1 was pruned out of G2
                    }
                }
            }
        }
            break;
        case 4:
        {
            const uint n1 = G1->getNumNodes();
            const uint n2 = G2->getNumNodes();
            vector<uint> reverse_A = vector<uint> (n2,n1);// value of n1 represents not used
            for(uint i=0; i< n1; i++){
                reverse_A[A[i]] = i;
            }
            for (const auto& edge: *(G2->getEdgeList())){
                node1 = edge[0], node2 = edge[1];
                if (MultiS3::numerator_type == MultiS3::ra_global){
                    ret += G2->getEdgeWeight(node1,node2) > 1 ? G2->getEdgeWeight(node1,node2) : 0;
                    if (reverse_A[node1] < n1 and reverse_A[node2] < n1 and G2->getEdgeWeight(node1,node2) > 0 and G1->getEdgeWeight(reverse_A[node1],reverse_A[node2]) == 1){
                        ret += G2->getEdgeWeight(node1,node2) > 1 ? 1:2;
                        // +1 because G1 was pruned out of G2
                    }
                }
            }
        }
            break;
        default:
        {
            for (const auto& edge: *(G1->getEdgeList()))
            {
               node1 = edge[0], node2 = edge[1];
               ret += G2->getEdgeWeight(A[node1],A[node2]) + 1; // +1 because G1 was pruned out of G2
               //cerr << "numerator_type not specified, Using default numerator." << endl;
            }
        }
            break;
    }
    
    return ret;
    
    
    
#else
    return 0;
#endif
}

uint MultiS3::computeDenom(const Alignment& A) const {
#ifdef MULTI_PAIRWISE
    uint ret = 0;
    
    uint node1, node2;
    const uint n1 = G1->getNumNodes();
    const uint n2 = G2->getNumNodes();
    vector<uint> whichPeg(n2, n1); // element equal to n1 represents not used/aligned
    for (uint i = 0; i < n1; ++i){
        whichPeg[A[i]] = i; // inverse of the alignment
    }
    switch (MultiS3::denominator_type){
        case 1:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (whichPeg[i] < n1 && whichPeg[j] < n1 && G1->getEdgeWeight(whichPeg[i],whichPeg[j]) > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G2->getEdgeWeight(i,j) > 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::rt_i){
                    if(whichPeg[i] < n1 && whichPeg[j] < n1){
                        ret += G1->getEdgeWeight(whichPeg[i],whichPeg[j]) + G2->getEdgeWeight(i,j);
                    }
                    else{
                        ret += G2->getEdgeWeight(i,j);
                    }
                }
            }
        }
            break;
        case 2:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (whichPeg[i] < n1 && whichPeg[j] < n1 && G1->getEdgeWeight(whichPeg[i],whichPeg[j]) > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G2->getEdgeWeight(i,j) > 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::ee_i) ret++;
            }
        }
            break;
        case 3:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (G2->getEdgeWeight(i,j) > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G1->getEdgeWeight(whichPeg[i],whichPeg[j]) > 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::ee_global) ret++;
            }
        }
            break;
        case 4:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (G2->getEdgeWeight(i,j) > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G1->getEdgeWeight(whichPeg[i],whichPeg[j]) > 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::rt_global){
                    if (whichPeg[i] < n1 && whichPeg[j] < n1){
                        ret += G1->getEdgeWeight(whichPeg[i],whichPeg[j]) + G2->getEdgeWeight(i,j);
                    }else{
                        ret +=  G2->getEdgeWeight(i,j);
                    }
                }
            }
        }
            break;
        default:
        {
	    //const uint n1 = G1->getNumNodes();
	    if (not degreesInit) initDegrees(A, *G1, *G2);
            for (uint i = 0; i < n1; i++) {
                if (shadowDegree[i] > 0) ret += n1-1-i; // Nil correctly compressed this from SANA1.1 but I think SANA1.1 was wrong
            } // Modified shadowDegree[i] ---> shadowDegree[A[i]]
            ret /= 2;
        }
            break;
    }
    return ret;
#else
    return;
#endif
}



double MultiS3::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    if (not degreesInit) initDegrees(A, *G1, *G2);


//    if (_type==1) denom = EdgeExposure::numExposedEdges(A, *G1, *G2);
//    else if (_type==0) setDenom(A);
    denom = computeDenom(A);
    uint correctNumer = computeNumer(A);
    if(correctNumer != numer) {
        cerr<<"inc eval MS3numer wrong: should be "<<correctNumer<<" but is "<<numer<<endl;
        numer = correctNumer;
    }
    return ((double) numer) / denom / NUM_GRAPHS;
#else
    return 0.0;
#endif
}

void MultiS3::initDegrees(const Alignment& A, const Graph& G1, const Graph& G2) {
    shadowDegree = vector<uint>(G2.getNumNodes(), 0);
    for (uint i = 0; i < G2.getNumNodes(); i++) shadowDegree[i] = G2.getNumNbrs(i);
    for (const auto& edge : *(G2.getEdgeList())) {
        auto w = G2.getEdgeWeight(edge[0],edge[1]);
        shadowDegree[edge[0]] += w; // doesn't this overcount by 1 since we already set it to getNumNbrs(i) above?
        if (edge[0] != edge[1]) shadowDegree[edge[1]] += w; //avoid double-counting for self-lopos
    }
    for (uint i = 0; i < G1.getNumNodes(); ++i) shadowDegree[A[i]] += 1;
    degreesInit = true;
}
