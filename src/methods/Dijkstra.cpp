#include <string>
#include <vector>

#include "Dijkstra.hpp"
#include "../utils/utils.hpp"

Dijkstra::Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC) :
    		Method(G1, G2, "Dijkstra_"+MC->toString()) {

	this->G1 = G1;
	this->G2 = G2;
	this->MC = MC;

	n1 = G1->getNumNodes();
	n2 = G2->getNumNodes();

	G1->getAdjMatrix(G1AdjMatrix);
	G2->getAdjMatrix(G2AdjMatrix);
	G1->getAdjLists(G1AdjLists);
	G2->getAdjLists(G2AdjLists);

	A = vector<ushort> (n1);

	sims = MC->getAggregatedLocalSims();
}


Alignment Dijkstra::run() {
	// Put all code here

	// sims[x][y] will give you how similar node x in G1 is to node y in G2 with 0 being the not at all similar

	// Put alignment into A[x] = y where x is an index of G1 and y is an index of G2

	return Alignment(A);
}


void Dijkstra::describeParameters(ostream& stream) {
	// Don't worry about this
}

string Dijkstra::fileNameSuffix(const Alignment& A) {
	return "_" + extractDecimals(MC->eval(A),3);
}

Dijkstra::~Dijkstra() {}

