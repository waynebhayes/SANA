#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include "Alignment.hpp"
#include "Graph.hpp"
#include "utils/utils.hpp"
using namespace std;

Alignment::Alignment(const vector<ushort>& mapping): A(mapping) {}

Alignment::Alignment(const Alignment& alig): A(alig.A) {}

Alignment::Alignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList) {
    map<string,ushort> mapG1 = G1->getNodeNameToIndexMap();
    map<string,ushort> mapG2 = G2->getNodeNameToIndexMap();
    ushort n1 = mapList.size();
    ushort n2 = G2->getNumNodes();
    A = vector<ushort>(G1->getNumNodes(), n2); //n2 used to note invalid value
    for (ushort i = 0; i < n1; i++) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];
        A[mapG1[nodeG1]] = mapG2[nodeG2];
    }
    printDefinitionErrors(*G1,*G2);
    assert(isCorrectlyDefined(*G1, *G2));
}

Alignment Alignment::loadEdgeList(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > edgeList(edges.size()/2, vector<string> (2));
    for (uint i = 0; i < edges.size()/2; i++) {
        edgeList[i][0] = edges[2*i];
        edgeList[i][1] = edges[2*i+1];
    }
    return Alignment(G1, G2, edgeList);
}

Alignment Alignment::loadPartialEdgeList(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > mapList(edges.size()/2, vector<string> (2));
    for (uint i = 0; i < edges.size()/2; i++) {
        mapList[i][0] = edges[2*i];
        mapList[i][1] = edges[2*i+1];
    }
    map<string,ushort> mapG1 = G1->getNodeNameToIndexMap();
    map<string,ushort> mapG2 = G2->getNodeNameToIndexMap();
    ushort n1 = G1->getNumNodes();
    ushort n2 = G2->getNumNodes();
    vector<ushort> A(n1, n2); //n2 used to note invalid value
    for (ushort i = 0; i < mapList.size(); i++) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];
        A[mapG1[nodeG1]] = mapG2[nodeG2];
    }
    vector<bool> G2AssignedNodes(n2, false);
    for (uint i = 0; i < n1; i++) {
        if (A[i] != n2) G2AssignedNodes[A[i]] = true;
    }
    for (uint i = 0; i < n1; i++) {
        if (A[i] == n2) {
            int j = randMod(n2);
            while (G2AssignedNodes[j]) j = randMod(n2);
            A[i] = j;
            G2AssignedNodes[j] = true;
        }
    }
    Alignment alig(A);
    assert(alig.isCorrectlyDefined(*G1, *G2));
    return alig;
}

Alignment Alignment::sanaOutloadEdgeList(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > edgeList;
    for (uint i = 0; i < edges.size()/2; i++) {
    	if(edges[2*i] == "end_of_edgelist") {
    		break; // "end_of_edgelist" separates sana.out's edge list from the analysis portion
    	}
    	vector<string> edge(2);

        edge[0] = edges[2*i];
        edge[1] = edges[2*i+1];

        edgeList.push_back(edge);
    }
    return Alignment(G1, G2, edgeList);
}
Alignment Alignment::loadMapping(string fileName) {
    if (not fileExists(fileName)) {
        throw runtime_error("Starting alignment file "+fileName+" not found");
    }
    ifstream infile(fileName.c_str());
    string line;
    getline(infile, line); //reads only the first line, ignores the rest
    istringstream iss(line);
    vector<ushort> A(0);
    int n;
    while (iss >> n) A.push_back(n);
    infile.close();
    return A;
}

Alignment Alignment::random(uint n1, uint n2) {
    //taken from: http://stackoverflow.com/questions/311703/algorithm-for-sampling-without-replacement
    vector<ushort> alignment(n1);
    ushort t = 0; // total input records dealt with
    ushort m = 0; // number of items selected so far
    double u;
    while (m < n1) {
        u = randDouble();
        if ((n2 - t)*u >= n1 - m) {
            t++;
        }
        else {
            alignment[m] = t;
            t++;
            m++;
        }
    }
    randomShuffle(alignment);
    return alignment;
}

Alignment Alignment::empty() {
    vector<ushort> emptyMapping(0);
    return Alignment(emptyMapping);
}

Alignment Alignment::identity(uint n) {
    vector<ushort> A(n);
    for (uint i = 0; i < n; i++) {
        A[i] = i;
    }
    return Alignment(A);
}

Alignment Alignment::correctMapping(const Graph& G1, const Graph& G2) {
    if (not G1.sameNodeNames(G2)) {
        throw runtime_error("cannot load correct mapping");
    }
    
    map<ushort,string> G1Index2Name = G1.getIndexToNodeNameMap();
    map<string,ushort> G2Name2Index = G2.getNodeNameToIndexMap();

    uint n = G1.getNumNodes();
    vector<ushort> A(n);
    for (uint i = 0; i < n; i++) {
        A[i] = G2Name2Index[G1Index2Name[i]];
    }
    return Alignment(A);
}

vector<ushort> Alignment::getMapping() const {
    return A;
}

ushort& Alignment::operator[] (ushort node) {
    return A[node];
}

const ushort& Alignment::operator[](const ushort node) const {
    return A[node];
}

uint Alignment::size() const {
    return A.size();
}

void Alignment::write(ostream& stream) const {
    for (uint i = 0; i < size(); i++) {
        stream << A[i] << " ";
    }
    stream << endl;
}

void Alignment::writeEdgeList(const Graph* G1, const Graph* G2, ostream& stream) const {
    map<ushort,string> mapG1 = G1->getIndexToNodeNameMap();
    map<ushort,string> mapG2 = G2->getIndexToNodeNameMap();
    for (uint i = 0; i < size(); i++) {
        stream << mapG1[i] << "\t" << mapG2[A[i]] << endl;
    }
}


uint Alignment::numAlignedEdges(const Graph& G1, const Graph& G2) const {
    vector<vector<ushort> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    vector<vector<bool> > G2AdjMatrix;
    G2.getAdjMatrix(G2AdjMatrix);

    uint count = 0;
    for (const auto& edge: G1EdgeList) {
        ushort node1 = edge[0], node2 = edge[1];
        count += G2AdjMatrix[A[node1]][A[node2]];
    }
    return count;
}

Graph Alignment::commonSubgraph(const Graph& G1, const Graph& G2) const {
    uint n = G1.getNumNodes();

    vector<vector<ushort> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    vector<vector<bool> > G2AdjMatrix;
    G2.getAdjMatrix(G2AdjMatrix);

    //only add edges preserved by alignment
    vector<vector<ushort> > edgeList(0);
    for (const auto& edge: G1EdgeList) {
        ushort node1 = edge[0], node2 = edge[1];
        if (G2AdjMatrix[A[node1]][A[node2]]) {
            edgeList.push_back(edge);
        }
    }
    return Graph(n, edgeList);
}

void Alignment::compose(const Alignment& other) {
    for (uint i = 0; i < size(); i++) {
        A[i] = other.A[A[i]];
    }
}

bool Alignment::isCorrectlyDefined(const Graph& G1, const Graph& G2) {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    vector<bool> G2AssignedNodes(n2, false);
    if (A.size() != n1) return false;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2 or G2AssignedNodes[A[i]]) return false;
        G2AssignedNodes[A[i]] = true;
    }
    return true;
}

void Alignment::printDefinitionErrors(const Graph& G1, const Graph& G2) {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    map<ushort,string> G1Names = G1.getIndexToNodeNameMap();
    map<ushort,string> G2Names = G2.getIndexToNodeNameMap();

    vector<bool> G2AssignedNodes(n2, false);
    int count = 0;
    if (A.size() != n1) cout<<"Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cout<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cout<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<" ("<<G2Names[A[i]]<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
    }
}
