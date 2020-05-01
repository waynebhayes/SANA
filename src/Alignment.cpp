#include "Alignment.hpp"
#include "Graph.hpp"
#include "utils/utils.hpp"

using namespace std;

Alignment::Alignment() {}
Alignment::Alignment(const vector<uint>& mapping): A(mapping) {}
Alignment::Alignment(const Alignment& alig): A(alig.A) {}
Alignment::Alignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList) {
    uint n1 = mapList.size();
    uint n2 = G2->getNumNodes();
    A = vector<uint>(G1->getNumNodes(), n2); //n2 used to note invalid value
    for (uint i = 0; i < n1; i++) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];
        A[G1->nodeNameToIndexMap[nodeG1]] = G2->nodeNameToIndexMap[nodeG2];
    }
    printDefinitionErrors(*G1,*G2);
    assert(isCorrectlyDefined(*G1, *G2));
}

uint Alignment::size() const { return A.size(); }
vector<uint> Alignment::asVector() const { return A; }
const vector<uint>* Alignment::getVector() const { return &A; }
uint& Alignment::back() { return A.back(); }
uint& Alignment::operator[] (uint node) { return A[node]; }
const uint& Alignment::operator[](const uint node) const { return A[node]; }

Alignment Alignment::loadEdgeList(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > edgeList(edges.size()/2, vector<string> (2));
    for (uint i = 0; i < edges.size()/2; i++) {
        edgeList[i][0] = edges[2*i];
        edgeList[i][1] = edges[2*i+1];
    }
    return Alignment(G1, G2, edgeList);
}

Alignment Alignment::loadEdgeListUnordered(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > edgeList(edges.size()/2, vector<string> (2));
    for (uint i = 0; i < edges.size()/2; i++){ //G1 is always edgeList[i][0], not 1.

        //check if G1 contains the nodename edges[2*i]. If this is false, G2 must contain
        //it and edges[2*i+1] must be in G1.
        if(find(G1->nodeNames.begin(), G1->nodeNames.end(), edges[2*i]) != G1->nodeNames.end()){
            edgeList[i][0] = edges[2*i];
            edgeList[i][1] = edges[2*i+1];
        }else{
            edgeList[i][0] = edges[2*i+1];
            edgeList[i][1] = edges[2*i];
        }
    }
    return Alignment(G1, G2, edgeList);
}

Alignment Alignment::loadPartialEdgeList(Graph* G1, Graph* G2, string fileName, bool byName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > mapList(edges.size()/2, vector<string> (2));
    for (uint i = 0; i < edges.size()/2; i++) {
        mapList[i][0] = edges[2*i];
        mapList[i][1] = edges[2*i+1];
    }
    const unordered_map<string,uint>* mapG1 = G1->getNodeNameToIndexMap();
    const unordered_map<string,uint>* mapG2 = G2->getNodeNameToIndexMap();
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    vector<uint> A(n1, n2); //n2 used to note invalid value
    for (uint i = 0; i < mapList.size(); i++) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];

        if (byName) {
            bool nodeG1Misplaced = false;
            bool nodeG2Misplaced = false;
            if (not mapG1->count(nodeG1)) {
                cout << nodeG1 << " not in G1 " << G1->getName();
                if (mapG2->count(nodeG1)) {
                    cout << ", but it is in G2. Will switch if appropriate." << endl;
                    nodeG1Misplaced = true;
                }else{
                    cout << endl;
                    continue;
                }
            }
            if (not mapG2->count(nodeG2)) {
                cout << nodeG2 << " not in G2 " << G2->getName();
                if (mapG1->count(nodeG2)){
                    cout << ", but it is in G1. Will switch if appropriate." << endl;
                    nodeG2Misplaced = true;
                } else {
                    cout << endl;
                    continue;
                }
            }
            if (nodeG1Misplaced and nodeG2Misplaced) {
                string temp = nodeG1;
                nodeG1 = nodeG2;
                nodeG2 = temp;
                cout << nodeG1 << " and " << nodeG2 << " swapped." << endl;
            }
            A[mapG1->at(nodeG1)] = mapG2->at(nodeG2);
        } else {
            A[atoi(nodeG1.c_str())] = atoi(nodeG2.c_str());
        }
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
    alig.printDefinitionErrors(*G1, *G2);
    assert(alig.isCorrectlyDefined(*G1, *G2));
    return alig;
}

Alignment Alignment::loadMapping(string fileName) {
    if (not fileExists(fileName)) {
        throw runtime_error("Starting alignment file "+fileName+" not found");
    }
    ifstream infile(fileName.c_str());
    string line;
    getline(infile, line); //reads only the first line, ignores the rest
    istringstream iss(line);
    vector<uint> A(0);
    int n;
    while (iss >> n) A.push_back(n);
    infile.close();
    return A;
}

Alignment Alignment::random(uint n1, uint n2) {
    //taken from: http://stackoverflow.com/questions/311703/algorithm-for-sampling-without-replacement
    vector<uint> alignment(n1);
    uint t = 0; // total input records dealt with
    uint m = 0; // number of items selected so far
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
    vector<uint> emptyMapping(0);
    return Alignment(emptyMapping);
}

Alignment Alignment::identity(uint n) {
    vector<uint> A(n);
    for (uint i = 0; i < n; i++) {
        A[i] = i;
    }
    return Alignment(A);
}

Alignment Alignment::correctMapping(const Graph& G1, const Graph& G2) {
    if (not G1.hasSameNodeNamesAs(G2)) {
        throw runtime_error("cannot load correct mapping; nodes have different names");
    }
    vector<uint> A(G1.getNumNodes());
    for (uint i = 0; i < G1.getNumNodes(); i++) {
        A[i] = G2.nodeNameToIndexMap.at(G1.nodeNames.at(i));
    }
    return Alignment(A);
}

Alignment &Alignment::operator=(Alignment other) {
    const uint n = max(A.size(), other.A.size());
    A.reserve(n);
    other.A.reserve(n);
    swap(A, other.A);
    A.shrink_to_fit();
    other.A.shrink_to_fit();
    return *this;
}

uint Alignment::numAlignedEdges(const Graph& G1, const Graph& G2) const {
    uint res = 0;
    for (const auto& edge: G1.edgeList)
        if (G2.hasEdge(A[edge[0]], A[edge[1]])) res++;
    return res;
}

Alignment Alignment::reverse(uint n2) const {
    uint n1 = size();
    vector<uint> A(n2, n1); //n1 used for invalid mapping
    for (uint i = 0; i < n1; i++) A[A[i]] = i;
    return Alignment(A);
}

void Alignment::compose(const Alignment& other) {
    for (uint i = 0; i < size(); i++) A[i] = other.A[A[i]];
}

bool Alignment::isCorrectlyDefined(const Graph& G1, const Graph& G2) {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    vector<bool> G2AssignedNodes(n2, false);
    vector<uint> colorMap = G1.myColorIdsToOtherGraphColorIds(G2);

    if (A.size() != n1) return false;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2 or G2AssignedNodes[A[i]]) return false;
        G2AssignedNodes[A[i]] = true;
        uint g1Color = G1.getNodeColor(i);
        uint g2Color = G2.getNodeColor(A[i]);
        if (colorMap.at(g1Color) != g2Color) return false;
    }
    return true;
}

void Alignment::printDefinitionErrors(const Graph& G1, const Graph& G2) {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    const vector<string>* G1Names = G1.getNodeNames();
    const vector<string>* G2Names = G2.getNodeNames();
    vector<bool> G2AssignedNodes(n2, false);
    vector<uint> colorMap = G1.myColorIdsToOtherGraphColorIds(G2);

    int count = 0;
    if (A.size() != n1) 
        cerr << "Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cerr<<count<<": node "<<i<<" ("<<(*G1Names)[i]<<") maps to ";
            cerr<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cerr<<count<<": node "<<i<<" ("<<(*G1Names)[i]<<") maps to "<<A[i]<<" (";
            cerr<<(*G2Names)[A[i]]<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
        uint g1Color = G1.getNodeColor(i);
        uint g2Color = G2.getNodeColor(A[i]);
        if (colorMap.at(g1Color) != g2Color) {
            string g1ColorName = G1.getColorName(g1Color);
            string g2ColorName = G2.getColorName(g2Color);
            cerr<<count<<": node "<<i<<" ("<<(*G1Names)[i]<<") of color ";
            cerr<<G1.getColorName(g1Color)<<" maps to "<<A[i]<<" (";
            cerr<<(*G2Names)[A[i]]<<") of color "<<G2.getColorName(g2Color)<<endl;
            count++;
        }
    }
}

//precondition: a valid color-restricted matching exists between G1 and G2
//that means: every color in G1 has at least as many nodes in G2
Alignment Alignment::randomColorRestrictedAlignment(const Graph& G1, const Graph& G2) {

    vector<uint> G2ColorIdToG1ColorId = G2.myColorIdsToOtherGraphColorIds(G1);
    vector<vector<uint>> G2NodesByG1ColorId(G1.numColors());
    for (uint node = 0; node < G2.getNumNodes(); node++) {
        uint g2ColorId = G2.getNodeColor(node);
        uint g1ColorId = G2ColorIdToG1ColorId[g2ColorId];
        if (g1ColorId == Graph::INVALID_COLOR_ID) continue;
        G2NodesByG1ColorId[g1ColorId].push_back(node);
    }
    for (uint id = 0; id < G1.numColors(); id++) {
        randomShuffle(G2NodesByG1ColorId[id]);
    }
    
    vector<uint> colorIndices(G1.numColors(), 0);
    vector<uint> A(0);
    A.reserve(G1.getNumNodes());
    for (uint node = 0; node < G1.getNumNodes(); node++) {
        uint colorId = G1.getNodeColor(node);
        if (colorIndices[colorId] >= G2NodesByG1ColorId[colorId].size()) {
            throw runtime_error("for some color, ran out of nodes in g2 to assign to nodes in g1");
        }
        A.push_back(G2NodesByG1ColorId[colorId][colorIndices[colorId]]);
        colorIndices[colorId]++;
    }

    Alignment alig(A);
    if (not alig.isCorrectlyDefined(G1, G2)) {
        alig.printDefinitionErrors(G1, G2);
        throw runtime_error("alignment not correctly defined");
    }
    return alig;   
}
