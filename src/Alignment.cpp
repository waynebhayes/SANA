#include "Alignment.hpp"
#include "Graph.hpp"
#include "utils/utils.hpp"
#include "utils/FileIO.hpp"
using namespace std;

Alignment::Alignment() = default;
Alignment::Alignment(const vector<uint>& mapping) {
    vector<atomic_uint> align(mapping.size());

    for (size_t i = 0; i < mapping.size(); ++i) {
        align[i].store(mapping[i]);
    }
    A = move(align);
}
Alignment::Alignment(const Alignment& alig){
    vector<atomic_uint> align(alig.A.size());

    for (size_t i = 0; i < alig.A.size(); ++i) {
        align[i].store(alig.A[i].load());
    }
    A = move(align);
};

Alignment::Alignment(const Graph& G1, const Graph& G2, const vector<array<string, 2>>& edgeList) {
    uint n1 = G1.getNumNodes(), n2 = G2.getNumNodes();
    assert(n1 == edgeList.size());
    A = vector<atomic_uint>(n1); //n2 used to denote invalid index
    for (auto &value: A) value.store(n2);
    for (const auto& edge : edgeList) {
        const string &nodeG1 = edge[0], &nodeG2 = edge[1];
        A[G1.getNameIndex(nodeG1)].store(G2.getNameIndex(nodeG2));
    }
    printDefinitionErrors(G1,G2);
    assert(isCorrectlyDefined(G1, G2));
}

Alignment Alignment::loadEdgeList(const Graph& G1, const Graph& G2, const string& fileName) {
    const vector<string> edges = FileIO::fileToWords(fileName);
    vector<array<string, 2>> edgeList;
    edgeList.reserve(edges.size()/2);
    for (uint i = 0; i < edges.size(); i += 2) {
        edgeList.push_back({edges[i], edges[i+1]});
    }
    return {G1, G2, edgeList};
}

Alignment Alignment::loadEdgeListUnordered(const Graph& G1, const Graph& G2, const string& fileName) {
    const vector<string> edges = FileIO::fileToWords(fileName);
    vector<array<string, 2>> edgeList;
    edgeList.reserve(edges.size()/2);
    for (uint i = 0; i < edges.size(); i += 2) {
        string name1 = edges[i], name2 = edges[i+1];
        //check if G1 contains a node named name1. If not, G2 must contain it and name2 must be in G1
        //(note: may not work as intended if graphs have overlapping names -Nil)
        if (G1.hasNodeName(name1)) {
            assert(G2.hasNodeName(name2));
            edgeList.push_back({name1, name2});
        } else {
            assert(G1.hasNodeName(name2));
            assert(G2.hasNodeName(name1));
            edgeList.push_back({name2, name1});
        }
    }
    return {G1, G2, edgeList};
}

Alignment Alignment::loadPartialEdgeList(const Graph& G1, const Graph& G2,
                                    const string& fileName, bool byName) {
    vector<string> edges = FileIO::fileToWords(fileName);
    vector<array<string, 2>> edgeList;
    edgeList.reserve(edges.size()/2);
    for (uint i = 0; i < edges.size(); i += 2) {
        edgeList.push_back({edges[i], edges[i+1]});
    }
    uint n1 = G1.getNumNodes(), n2 = G2.getNumNodes();
    vector<uint> A(n1, n2); //n2 used to note invalid value
    for (const auto& edge : edgeList) {
        string nodeG1 = edge[0], nodeG2 = edge[1];
        if (not byName) {
            A[stoi(nodeG1)] = stoi(nodeG2);
        } else {
            bool nodeG1Misplaced = false;
            bool nodeG2Misplaced = false;
            if (not G1.hasNodeName(nodeG1)) {
                cout << nodeG1 << " not in G1 " << G1.getName();
                if (G2.hasNodeName(nodeG1)) {
                    cout << ", but it is in G2. Will switch if appropriate." << endl;
                    nodeG1Misplaced = true;
                } else {
                    cout << endl;
                    continue;
                }
            }
            if (not G2.hasNodeName(nodeG2)) {
                cout << nodeG2 << " not in G2 " << G2.getName();
                if (G1.hasNodeName(nodeG2)) {
                    cout << ", but it is in G1. Will switch if appropriate." << endl;
                    nodeG2Misplaced = true;
                } else {
                    cout << endl;
                    continue;
                }
            }
            if (nodeG1Misplaced and nodeG2Misplaced) {
                std::swap(nodeG1, nodeG2);
                cout << nodeG1 << " and " << nodeG2 << " swapped." << endl;
            }
            A[G1.getNameIndex(nodeG1)] = G2.getNameIndex(nodeG2);
        }
    }
    vector<bool> G2AssignedNodes(n2, false);
    for (uint i = 0; i < n1; i++) {
        if (A[i] != n2) {
            if (G2AssignedNodes[A[i]]) throw runtime_error("two G1 nodes map to the same G2 node");
            G2AssignedNodes[A[i]] = true;
        }
    }
    for (uint i = 0; i < n1; i++) {
        if (A[i] == n2) {
            unsigned j = randIndex(n2);
            while (G2AssignedNodes[j]) j = randIndex(n2);
            A[i] = j;
            G2AssignedNodes[j] = true;
        }
    }
    Alignment alig(A);
    alig.printDefinitionErrors(G1, G2);
    assert(alig.isCorrectlyDefined(G1, G2));
    return alig;
}

Alignment Alignment::loadMapping(const string& fileName) {
    if (not FileIO::fileExists(fileName)) {
        throw runtime_error("Starting alignment file "+fileName+" not found");
    }
    ifstream ifs(fileName);
    string firstLine;
    FileIO::safeGetLine(ifs, firstLine); //ignore anything past first line
    istringstream iss(firstLine);
    vector<uint> A(0);
    int g2Ind;
    while (iss >> g2Ind) A.push_back(g2Ind);
    return A;
}

Alignment Alignment::random(uint n1, uint n2) {
    //taken from: http://stackoverflow.com/questions/311703/algorithm-for-sampling-without-replacement
    vector<uint> alignment(n1);
    uint t = 0; // total input records dealt with
    uint m = 0; // number of items selected so far
    while (m < n1) {
        double u = randDouble();
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
    return {emptyMapping};
}

Alignment Alignment::identity(uint n) {
    vector<uint> A(n);
    for (uint i = 0; i < n; i++) {
        A[i] = i;
    }
    return {A};
}

Alignment Alignment::correctMapping(const Graph& G1, const Graph& G2) {
    if (not G1.hasSameNodeNamesAs(G2)) {
        throw runtime_error("cannot load correct mapping; nodes have different names");
    }
    vector<uint> A(G1.getNumNodes());
    for (uint i = 0; i < G1.getNumNodes(); i++) {
        A[i] = G2.getNameIndex(G1.getNodeName(i));
    }
    return {A};
}

Alignment &Alignment::operator=(Alignment other) {
    std::swap(A, other.A);
    return *this;
}

uint Alignment::computeNumAlignedEdges(const Graph& G1, const Graph& G2) const {
    uint res = 0;
    // Note this NEEDS TO STAY THE WAY IT IS for MULTI to work correctly, even though it's badly named for other cases.
    // This is because in MULTI, G2 is the shadow network, and G2.getEdgeWeight tells us how many edges from the other
    // networks are "above" the shadow network. For almost any other case, this function is really mis-named and should,
    // in principle, use "hasEdge" if we wanted to actually return the *number* of aligned edges. But who cares, because....
    // NOTE2: this really shouldn't be used in objective functions, because different objective functions can make
    // arbitrary specifications on what an aligned edge costs. For example if the edges are weighted, then EdgeRatio
    // says the aligned edges is min(e1,e2)/max(e1,e2), whereas EdgeMin is just min(e1,e2). In such cases "counting"
    // aligned edges is irrelevant.
    for (const auto& edge: *(G1.getEdgeList()))
        res += G2.getEdgeWeight(A[edge[0]], A[edge[1]]);
    return res;
}

Alignment Alignment::reverse(uint n2) const {
    uint n1 = size();
    vector<uint> A(n2, n1); //n1 used for invalid mapping
    for (uint i = 0; i < n1; i++) A[this->A[i].load()] = i;
    return {A};
}

void Alignment::compose(const Alignment& other) {
    for (uint i = 0; i < size(); i++) A[i].store(other.A[A[i].load()].load());
}

bool Alignment::isCorrectlyDefined(const Graph& G1, const Graph& G2) const {
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

void Alignment::printDefinitionErrors(const Graph& G1, const Graph& G2) const {
    uint n1 = G1.getNumNodes(), n2 = G2.getNumNodes();
    vector<bool> G2AssignedNodes(n2, false);
    vector<uint> colorMap = G1.myColorIdsToOtherGraphColorIds(G2);
    int count = 0;
    if (A.size() != n1) 
        cerr << "Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cerr<<count<<": node "<<i<<" ("<<G1.getNodeName(i)<<") maps to ";
            cerr<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cerr<<count<<": node "<<i<<" ("<<G1.getNodeName(i)<<") maps to "<<A[i]<<" (";
            cerr<<G2.getNodeName(A[i])<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
        uint g1Color = G1.getNodeColor(i);
        uint g2Color = G2.getNodeColor(A[i]);
        if (colorMap.at(g1Color) != g2Color) {
            string g1ColorName = G1.getColorName(g1Color);
            string g2ColorName = G2.getColorName(g2Color);
            cerr<<count<<": node "<<i<<" ("<<G1.getNodeName(i)<<") of color ";
            cerr<<G1.getColorName(g1Color)<<" maps to "<<A[i]<<" (";
            cerr<<G2.getNodeName(A[i])<<") of color "<<G2.getColorName(g2Color)<<endl;
            count++;
        }
    }
}

//precondition: a valid color-restricted matching exists between G1 and G2
//equivalently: every color in G1 has at least as many nodes in G2
Alignment Alignment::randomColorRestrictedAlignment(const Graph& G1, const Graph& G2) {
    vector<uint> g2ColIdToG1ColId = G2.myColorIdsToOtherGraphColorIds(G1);
    vector<vector<uint>> g1ColIdToG2Nodes(G1.numColors());
    for (uint g2Node = 0; g2Node < G2.getNumNodes(); g2Node++) {
        uint g2ColId = G2.getNodeColor(g2Node);
        uint g1ColId = g2ColIdToG1ColId[g2ColId];
        if (g1ColId == Graph::INVALID_COLOR_ID) continue;
        g1ColIdToG2Nodes[g1ColId].push_back(g2Node);
    }
    for (uint g1ColId = 0; g1ColId < G1.numColors(); g1ColId++) {
        randomShuffle(g1ColIdToG2Nodes[g1ColId]);
    }
    
    vector<uint> A(0);
    A.reserve(G1.getNumNodes());
    for (uint g1Node = 0; g1Node < G1.getNumNodes(); g1Node++) {
        uint g1ColId = G1.getNodeColor(g1Node);
        if (g1ColIdToG2Nodes[g1ColId].empty()) {
            throw runtime_error("not enough nodes in G2 with color "+G1.getColorName(g1ColId));
        }
        A.push_back(g1ColIdToG2Nodes[g1ColId].back());
        g1ColIdToG2Nodes[g1ColId].pop_back();
    }

    Alignment alig(A);
    if (not alig.isCorrectlyDefined(G1, G2)) {
        alig.printDefinitionErrors(G1, G2);
        throw runtime_error("alignment not correctly defined");
    }
    return alig;   
}
