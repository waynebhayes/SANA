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
    cerr << "Start execution of " << name << endl;
    Timer T;
    T.start();
    Alignment A = run();
    execTime = T.elapsed();
    cerr << "Executed " << name << " in " << T.elapsedString() << endl;
    checkLockingBeforeReport(A);
    return A;
}

// Stops the program if locked nodes have been moved
void  Method::checkLockingBeforeReport(Alignment A){
	// No locking
	if(G1->getLockedCount() == 0)
		return;

	map<ushort,string> g1_NameMap = G1->getIndexToNodeNameMap();
	map<string,ushort> g2_IndexMap = G2->getNodeNameToIndexMap();
	map<ushort,string> g2_NameMap = G2->getIndexToNodeNameMap();

	uint n1 = G1->getNumNodes();
	for(uint i=0; i< n1 ; i++){
		if(!G1->isLocked(i))
			continue;
		string node1 = g1_NameMap[i];
		string node2 = G1->getLockedTo(i);
		uint pairIndex = g2_IndexMap[node2];
		if(A[i] != pairIndex){
			cerr << A[i] << " != " << pairIndex << " <---> ";
			cerr << g2_NameMap[A[i]] << " != "<< node2 << endl;
			throw runtime_error(node1 + " is not locked to " + node2);
		}
	}


//	if(lockFileName != "" and fileExists(lockFileName)){
//		checkFileExists(lockFileName);
//		map<string,ushort> g1_IndexMap = G1->getNodeNameToIndexMap();
//		map<string,ushort> g2_IndexMap = G2->getNodeNameToIndexMap();
//
//		map<ushort,string> g1_NameMap = G1->getIndexToNodeNameMap();
//		map<ushort,string> g2_NameMap = G2->getIndexToNodeNameMap();
//
//		ifstream ifs(lockFileName);
//		string node1, node2;
//		while(ifs >> node1 >> node2){
//			if(A[g1_IndexMap[node1]] != g2_IndexMap[node2]){
//				cerr << A[g1_IndexMap[node1]] << " != " << g2_IndexMap[node2] << endl;
//				cerr <<	g2_NameMap[A[g1_IndexMap[node1]]] << " != " << node2 << endl;
//				throw runtime_error(node1 + " is not locked to " + node2);
//			}
//		}
//	}
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
			cerr << "Lock file (" << fileName << ") does not exists!" << endl;
			throw runtime_error("Lock file not found: " + fileName);
		}
	}
}
