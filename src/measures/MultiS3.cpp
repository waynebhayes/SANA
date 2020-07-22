#include "MultiS3.hpp"
#include "EdgeExposure.hpp"
#include <cmath>
#include <iostream>

uint NUM_GRAPHS;
double MultiS3::Normalization_factor = 2;
uint MultiS3::denom = 1;
uint MultiS3::numer = 1;
double MultiS3::_type = 0;
bool MultiS3::degreesInit = false;
vector<uint> MultiS3::shadowDegree;

unsigned MultiS3::numerator_type   = 0; // 0 for default version of ms3
unsigned MultiS3::denominator_type = 0;


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
        case MultiS3::ra_i:
            cout<<"ra_i"<<endl;
            break;
        case MultiS3::la_i:
            cout<<"la_i"<<endl;
            break;
        case MultiS3::la_global:
            cout<<"la_global"<<endl;
            break;
        case MultiS3::ra_global:
            cout<<"ra_global"<<endl;
            break;
        default:
            cout<<"default"<<endl;
    }
    
    cout<<"MultiS3: denom_type = ";
    switch(denominator_type){
        case MultiS3::rt_i:
            cout<<"rt_i"<<endl;
            break;
        case MultiS3::ee_i:
            cout<<"ee_i"<<endl;
            break;
        case MultiS3::ee_global:
            cout<<"ee_global"<<endl;
            break;
        case MultiS3::rt_global:
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

void MultiS3::getNormalizationFactor() const {
#ifdef MULTI_PAIRWISE
    double factor_denom = 0;
    uint node1, node2;
    switch (MultiS3::denominator_type) {
        case MultiS3::rt_i: 
        {
            /*for (const auto& edge: *(G2->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                factor_denom += G2->getEdgeWeight(node1,node2) + 1;
            }*/
            factor_denom = G1->getEdgeList()->size() * NUM_GRAPHS;
        }
            break;
        case MultiS3::ee_i:
        {
            factor_denom = G2->getEdgeList()->size() + G1->getEdgeList()->size();
        }
            break;
        case MultiS3::ee_global:
        {
            factor_denom = G2->getEdgeList()->size() + G1->getEdgeList()->size();
        }
            break;
        case MultiS3::rt_global:
        {
            for (const auto& edge: *(G2->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                factor_denom += G2->getEdgeWeight(node1,node2) + 1;
            }
        }
            break;
    }
    //cout << "Factor denom is "<< factor_denom << endl;
    switch (MultiS3::numerator_type){
        case MultiS3::ra_i:
        {
            Normalization_factor = G1->getEdgeList()->size() * NUM_GRAPHS / factor_denom;
        }
            break;
        case MultiS3::la_i:
        {
            Normalization_factor = G1->getEdgeList()->size() / factor_denom;
        }
            break;
        case MultiS3::la_global:
        {
            uint temp_max = 0;
            for (const auto& edge: *(G2->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                temp_max = G2->getEdgeWeight(node1,node2) + 1 > temp_max ? G2->getEdgeWeight(node1,node2) + 1 : temp_max;
            }
            Normalization_factor = temp_max * G2->getEdgeList()->size() / factor_denom / 2;
        }
            break;
        case MultiS3::ra_global:
        {
            uint temp_max = 0;
            for (const auto& edge: *(G2->getEdgeList()))
            {
                node1 = edge[0], node2 = edge[1];
                temp_max = G2->getEdgeWeight(node1,node2) + 1 > temp_max ? G2->getEdgeWeight(node1,node2) + 1 : temp_max;
            }
            Normalization_factor = temp_max * G2->getEdgeList()->size() / factor_denom;
        }
            break;
    }
    //cout << "Normalization factor is " << Normalization_factor << endl;
#else
return;
#endif
}


uint MultiS3::computeNumer(const Alignment& A) const {
#ifdef MULTI_PAIRWISE
    uint ret = 0;
    uint node1, node2;

    switch (MultiS3::numerator_type){
        case MultiS3::ra_i:
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
        case MultiS3::la_i:
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
        case MultiS3::la_global:
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
        case MultiS3::ra_global:
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
    uint ret = 0;
#ifdef MULTI_PAIRWISE
    
    uint node1, node2;
    const uint n1 = G1->getNumNodes();
    const uint n2 = G2->getNumNodes();
    vector<uint> whichPeg(n2, n1); // element equal to n1 represents not used/aligned
    for (uint i = 0; i < n1; ++i){
        whichPeg[A[i]] = i; // inverse of the alignment
    }
    switch (MultiS3::denominator_type){
        case MultiS3::rt_i:
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
        case MultiS3::ee_i:
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
        case MultiS3::ee_global:
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
        case MultiS3::rt_global:
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
#endif
    return ret;
}



double MultiS3::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    getNormalizationFactor();
    if (not degreesInit) initDegrees(A, *G1, *G2);


//    if (_type==1) denom = EdgeExposure::numExposedEdges(A, *G1, *G2);
//    else if (_type==0) setDenom(A);
    denom = computeDenom(A);
    uint correctNumer = computeNumer(A);
    if(correctNumer != numer) {
        cerr<<"inc eval MS3numer wrong: should be "<<correctNumer<<" but is "<<numer<<endl;
        numer = correctNumer;
    }
    return ((double) numer) / denom / Normalization_factor;//NUM_GRAPHS;
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

