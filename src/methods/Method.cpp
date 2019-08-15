#include "Method.hpp"
#include <cassert>
#include <string>

Method::Method(Graph* G1, Graph* G2, string name): G1(G1), G2(G2), name(name) {};

Method::~Method() {
}

string Method::getName() {
    return name;
}

Alignment Method::runAndPrintTime() {
    cout << "Start execution of " << name << endl;
    Timer T;
    T.start();
    Alignment A = run();
    execTime = T.elapsed();
    cout << "Executed " << name << " in " << T.elapsedString() << endl;

#ifdef REINDEX
    // Re Index back to normal (Method #3 of locking)
    Timer T2;
    T2.start();

    if(G1->isBipartite()){
        G1->reIndexGraph(getReverseMap(G1->getBipartiteNodeTypes_ReIndexMap()));
        A.reIndexAfter_Iterations(G1->getBipartiteNodeTypes_ReIndexMap());
    }
    // if locking is enabled but hasnodeType is not
    else if(G1->getLockedCount() > 0){
         G1->reIndexGraph(getReverseMap(G1->getLocking_ReIndexMap()));
          A.reIndexAfter_Iterations(G1->getLocking_ReIndexMap());
    }
    cout << "ReIndex to normal took " << T2.elapsedString() << endl;
#endif    
    checkNodeTypesBeforeReport(A);
    checkLockingBeforeReport(A);
    return A;
}

// Stops the program if locked nodes have been moved
void  Method::checkLockingBeforeReport(Alignment A){
    // No locking
    if(G1->getLockedCount() == 0)
        return;

    unordered_map<uint,string> g1_NameMap = G1->getIndexToNodeNameMap();
    unordered_map<string,uint> g2_IndexMap = G2->getNodeNameToIndexMap();
    unordered_map<uint,string> g2_NameMap = G2->getIndexToNodeNameMap();

    uint n1 = G1->getNumNodes();
    for(uint i=0; i< n1 ; i++){
        if(!G1->isLocked(i))
            continue;
        string node1 = g1_NameMap[i];
        string node2 = G1->getLockedTo(i);
        uint pairIndex = g2_IndexMap[node2];
        if(A[i] != pairIndex){
            cout << A[i] << " != " << pairIndex << " <---> ";
            cout << g2_NameMap[A[i]] << " != "<< node2 << endl;
            throw runtime_error(node1 + " is not locked to " + node2);
        }
    }
}

void Method::checkNodeTypesBeforeReport(Alignment A){
    if(!G1->isBipartite())
        return;

    unordered_map<uint,string> g1_NameMap = G1->getIndexToNodeNameMap();
    unordered_map<uint,string> g2_NameMap = G2->getIndexToNodeNameMap();

    uint n1 = G1->getNumNodes();
    for(uint i=0; i< n1 ; i++){
        if(G1->getBipartiteNodeType(i) != G2->getBipartiteNodeType(A[i])){
            cout << g1_NameMap[i] << "( " << G1->getBipartiteNodeType(i) << " ) should not be aligned to " <<
                 g2_NameMap[A[i]] << "( " << G2->getBipartiteNodeType(A[i]) << " )" << endl;
            throw runtime_error(g1_NameMap[i] + "( " + to_string(G1->getBipartiteNodeType(i)) + " ) should not be aligned to "
                    + g2_NameMap[A[i]] + "( " + to_string(G2->getBipartiteNodeType(A[i])) + " )");
        }
    }
}


double Method::getExecTime() {
    return execTime;
}

void Method::setLockFile(string fileName){
    if(fileName != "")
    {
        assert(implementsLocking());
        if(fileExists(fileName)){
            lockFileName = fileName;
        }
        else{
            cout << "Lock file (" << fileName << ") does not exists!" << endl;
            throw runtime_error("Lock file not found: " + fileName);
        }
    }
}


