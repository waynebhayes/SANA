#include "NodeCorrectness.hpp"
#include <vector>
#include <iostream>

#define TRUEALIGNMENT_ERRORCHECKING

namespace
{
    void displayWarnings(bool oneToOneError, bool nodeExistError, unsigned int E, unsigned int R)
    {

        if(oneToOneError)
        {
        std::cout << "WARNING: PROVIDED TRUE ALIGNMENT IS NOT ONE TO ONE" << std::endl;
        std::cout << "         ONLY ONE TO ONE PAIRS HAVE BEEN INCLUDED" << std::endl;
        }
        if ( nodeExistError )
        {

        std::cout << "WARNING: TRUE ALIGNMENT CONTAINS NODES WHICH DO NOT EXIST" << std::endl;
            std::cout << "         ONLY PAIRS WITH NODES WHICH EXIST HAVE BEEN INCLUDED" << std::endl;
        }
        if( oneToOneError || nodeExistError)
        {
        std::cout << "         ORIGINAL TRUEALIGNMENT SIZE = " << E << std::endl;
        std::cout << "         ACTUAL TRUEALIGNMENT SIZE = " << R << std::endl;
        }
    }
}

using namespace std;

NodeCorrectness::NodeCorrectness(const Alignment& A): Measure(NULL, NULL, "nc"),
    trueA(A) {
}

NodeCorrectness::~NodeCorrectness() {
}

double NodeCorrectness::eval(const Alignment& A) {
    uint count = 0;
    for (uint i = 0; i < A.size(); i++) {
        if (A[i] == trueA[i]) count++;
    }
//    std::cout << count << "/" << trueA.getBack() << std::endl;
    return (double) count / trueA.getBack();
}

vector<uint> NodeCorrectness::convertAlign(const Graph& G1, const Graph& G2, const vector<string>& E){
    //necessary variables
    unordered_map<string,uint> mapG1 = G1.getNodeNameToIndexMap();
    unordered_map<string,uint> mapG2 = G2.getNodeNameToIndexMap();
    vector<uint> alignment(G1.getNumNodes(), G2.getNumNodes());
    //this is the initial size of the provided true alignment file
    alignment.push_back(E.size()/2);

    bool exist = true;
    bool used = false;
    bool oneToOneError = false;
    bool nodeExistError = false;
    bool g2Used[G2.getNumNodes()];

    #ifdef TRUEALIGNMENT_ERRORCHECKING
    for(unsigned int i = 0; i < G2.getNumNodes(); ++i)
    g2Used[i] = false;
    #endif
    for(unsigned int i = 0; i < E.size()/2; ++i)
    {
        string n1 = E[2*i];
    string n2 = E[2*i+1];
    unsigned int g1pos,g2pos;
    #ifdef TRUEALIGNMENT_ERRORCHECKING
        try{
    //checks for nodes which do not exist; if either one or both nodes
    //fail to exist, then that pair is ignored and the size of the
    //true alignment is reduced by one
        g1pos = mapG1.at(n1);
        g2pos = mapG2.at(n2);
    }catch(...){
        alignment.back()--;
        exist = false;
        nodeExistError = true;
    }
    //only if the nodes exist, which they be checked if they have already
    //been used; if they have the true alignment is not 1-1 and the pair
    //is not counted in the truealignment size
    if(exist && (alignment[g1pos] != G2.getNumNodes() || g2Used[g2pos]))
    {
        alignment.back()--;
        used = true;
        oneToOneError = true;
    }
    #endif
    #ifndef TRUEALIGNMENT_ERRORCHECKING
        g1pos = mapG1[n1];
        g2pos = mapG2[n2];
    #endif

        //only if the nodes exist and the pair keeps truealignment 1 to 1,
    //then it will added to the true alignment vector
    //this always happens if errorchecking is off
    if( exist && !used)
    {
        g2Used[g2pos] = true;
        alignment[mapG1.at(n1)] = mapG2.at(n2);
    }
        exist = true;
        used = false;
    }
    displayWarnings(oneToOneError, nodeExistError, E.size()/2, alignment.back());

    return alignment;
}

vector<uint> NodeCorrectness::getMappingforNC() const{
    return trueA.getMapping();
}

bool NodeCorrectness::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return G1->sameNodeNames(*G2);
}
