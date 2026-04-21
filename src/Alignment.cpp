#include "Alignment.hpp"
#include "Graph.hpp"
#include "measures/EdgeCorrectness.hpp"
#include "utils/utils.hpp"
#include "utils/FileIO.hpp"
using namespace std;

Alignment::Alignment():
pegNum(0), holeNum(0), pegsToHoles(0), holesToPegs(0) {}

Alignment::Alignment(const Alignment& other):
    pegNum(other.pegNum),
    holeNum(other.holeNum) {
    pegsToHoles = move(vector<atomic_uint>(pegNum));
    holesToPegs = move(vector<atomic_uint>(holeNum));
    for (size_t peg = 0; peg < pegNum; ++peg) {
        pegsToHoles[peg].store(other.pegsToHoles[peg].load(memory_order_relaxed), memory_order_relaxed);
    }
    for (size_t hole = 0; hole < holeNum; ++hole) {
        holesToPegs[hole].store(other.holesToPegs[hole].load(memory_order_relaxed), memory_order_relaxed);
    }
}

Alignment &Alignment::operator=(Alignment other) {
    std::swap(pegsToHoles, other.pegsToHoles);
    std::swap(holesToPegs, other.holesToPegs);
    pegNum = other.pegNum;
    holeNum = other.holeNum;
    return *this;
}

Alignment::Alignment(const vector<uint>& mapping, uint holeNum):
    pegNum(mapping.size()),
    holeNum(holeNum) {
    pegsToHoles = move(vector<atomic_uint>(mapping.size()));
    holesToPegs = move(vector<atomic_uint>(holeNum));

    for (auto &holesPeg: holesToPegs) holesPeg.store(invalidPeg, memory_order_relaxed);

    for (size_t peg = 0; peg < mapping.size(); ++peg) {
        uint hole = mapping[peg];
        pegsToHoles[peg].store(hole, memory_order_relaxed);
        holesToPegs[hole].store(peg, memory_order_relaxed);
    }
}

Alignment::Alignment(const Graph& G1, const Graph& G2, const vector<array<string, 2>>& edgeList):
    pegNum(G1.getNumNodes()),
    holeNum(G2.getNumNodes()) {
    assert(pegNum == edgeList.size());

    pegsToHoles = move(vector<atomic_uint>(pegNum));
    holesToPegs = move(vector<atomic_uint>(holeNum));
    for (auto &pegsHole: pegsToHoles) pegsHole.store(invalidHole, memory_order_relaxed);
    for (auto &holesPeg: holesToPegs) holesPeg.store(invalidPeg, memory_order_relaxed);

    for (const auto& edge : edgeList) {
        const string &pegName = edge[0], &holeName = edge[1];
        uint peg = G1.getNameIndex(pegName);
        uint hole = G2.getNameIndex(holeName);
        pegsToHoles[peg].store(hole, memory_order_relaxed);
        holesToPegs[hole].store(peg, memory_order_relaxed);
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
    return Alignment(G1, G2, edgeList);
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
    return Alignment(G1, G2, edgeList);
}

Alignment Alignment::loadPartialEdgeList(const Graph& G1, const Graph& G2, const string& fileName, bool byName) {
    vector<string> edges = FileIO::fileToWords(fileName);
    vector<array<string, 2>> edgeList;
    edgeList.reserve(edges.size()/2);
    for (size_t i = 0; i < edges.size(); i += 2) {
        edgeList.push_back({edges[i], edges[i+1]});
    }
    uint pegNum = G1.getNumNodes();
    uint holeNum = G2.getNumNodes();

    Alignment newAlignment;
    newAlignment.pegNum = pegNum;
    newAlignment.holeNum = holeNum;
    newAlignment.pegsToHoles = move(vector<atomic_uint>(pegNum));
    newAlignment.holesToPegs = move(vector<atomic_uint>(holeNum));

    vector<atomic_uint> &pegsToHoles = newAlignment.pegsToHoles;
    vector<atomic_uint> &holesToPegs = newAlignment.holesToPegs;
    for (auto &holesPeg: holesToPegs) holesPeg.store(invalidPeg, memory_order_relaxed);

    for (const auto& edge : edgeList) {
        const string *pegName = &edge[0], *holeName = &edge[1];
        if (not byName) {
            uint peg = stoul(*pegName);
            uint hole = stoul(*holeName);
            pegsToHoles[peg].store(hole, memory_order_relaxed);
            holesToPegs[hole].store(peg, memory_order_relaxed);
        } else {
            bool nodeG1Misplaced = false;
            bool nodeG2Misplaced = false;
            if (not G1.hasNodeName(*pegName)) {
                cout << pegName << " not in G1 " << G1.getName();
                if (G2.hasNodeName(*pegName)) {
                    cout << ", but it is in G2. Will switch if appropriate." << endl;
                    nodeG1Misplaced = true;
                } else {
                    cout << endl;
                    continue;
                }
            }
            if (not G2.hasNodeName(*holeName)) {
                cout << holeName << " not in G2 " << G2.getName();
                if (G1.hasNodeName(*holeName)) {
                    cout << ", but it is in G1. Will switch if appropriate." << endl;
                    nodeG2Misplaced = true;
                } else {
                    cout << endl;
                    continue;
                }
            }
            if (nodeG1Misplaced and nodeG2Misplaced) {
                std::swap(pegName, holeName);
                cout << pegName << " and " << holeName << " swapped." << endl;
            }
            uint peg = G1.getNameIndex(*pegName);
            uint hole = G2.getNameIndex(*holeName);
            pegsToHoles[peg].store(hole, memory_order_relaxed);
            holesToPegs[hole].store(peg, memory_order_relaxed);
        }
    }
    for (uint peg = 0; peg < pegNum; peg++) {
        const uint hole = pegsToHoles[peg].load(memory_order_relaxed);
        if (hole != invalidHole) {
            if (holesToPegs[hole].load(memory_order_relaxed) != peg) {
                throw runtime_error("two G1 nodes map to the same G2 node");
            }
        }
    }
    for (uint peg = 0; peg < pegNum; peg++) {
        if (pegsToHoles[peg] == holeNum) {
            uint hole = randIndex(holeNum);
            while (holesToPegs[hole] != invalidPeg) {
                hole = randIndex(holeNum);
            }
            pegsToHoles[peg].store(hole, memory_order_relaxed);
            holesToPegs[hole].store(peg, memory_order_relaxed);
        }
    }
    newAlignment.printDefinitionErrors(G1, G2);
    assert(newAlignment.isCorrectlyDefined(G1, G2));
    return newAlignment;
}

Alignment Alignment::loadMapping(const string& fileName, const Graph& G1, const Graph& G2) {
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
    return Alignment(A, G2.getNumNodes());
}


//precondition: a valid color-restricted matching exists between G1 and G2
//equivalently: every color in G1 has at least as many nodes in G2
Alignment Alignment::randomColorRestrictedAlignment(const Graph& G1, const Graph& G2) {

    const vector<uint> holeColorToPegColor = G2.myColorIdsToOtherGraphColorIds(G1);
    vector<vector<uint>> pegColorToHoles(G1.numColors());
    for (uint hole = 0; hole < G2.getNumNodes(); hole++) {
        uint holeColor = G2.getNodeColor(hole);
        uint pegColor = holeColorToPegColor[holeColor];
        if (pegColor == Graph::INVALID_COLOR_ID) continue;
        pegColorToHoles[pegColor].push_back(hole);
    }
    for (uint pegColor = 0; pegColor < G1.numColors(); pegColor++) {
        randomShuffle(pegColorToHoles[pegColor]);
    }

    uint pegNum = G1.getNumNodes();
    uint holeNum = G2.getNumNodes();

    Alignment newAlignment;
    newAlignment.pegNum = pegNum;
    newAlignment.holeNum = holeNum;
    newAlignment.pegsToHoles = move(vector<atomic_uint>(pegNum));
    newAlignment.holesToPegs = move(vector<atomic_uint>(holeNum));

    vector<atomic_uint> &pegsToHoles = newAlignment.pegsToHoles;
    vector<atomic_uint> &holesToPegs = newAlignment.holesToPegs;
    for (auto &holesPeg: holesToPegs) holesPeg.store(invalidPeg, memory_order_relaxed);

    for (uint peg = 0; peg < pegNum; peg++) {
        uint pegColor = G1.getNodeColor(peg);
        if (pegColorToHoles[pegColor].empty()) {
            throw runtime_error("not enough nodes in G2 with color "+G1.getColorName(pegColor));
        }
        uint hole = pegColorToHoles[pegColor].back();
        pegColorToHoles[pegColor].pop_back();

        pegsToHoles[peg].store(hole, memory_order_relaxed);
        holesToPegs[hole].store(peg, memory_order_relaxed);
    }

    if (not newAlignment.isCorrectlyDefined(G1, G2)) {
        newAlignment.printDefinitionErrors(G1, G2);
        throw runtime_error("alignment not correctly defined");
    }
    return newAlignment;
}

Alignment Alignment::random(uint pegNum, uint holeNum) {
    //taken from: http://stackoverflow.com/questions/311703/algorithm-for-sampling-without-replacement
    vector<uint> alignment(pegNum);
    uint t = 0; // total input records dealt with
    uint m = 0; // number of items selected so far
    while (m < pegNum) {
        double u = randDouble();
        if ((holeNum - t)*u >= pegNum - m) {
            t++;
        }
        else {
            alignment[m] = t;
            t++;
            m++;
        }
    }
    randomShuffle(alignment);

    return Alignment(alignment, holeNum);
}

Alignment Alignment::empty() {
    return Alignment();
}

Alignment Alignment::identity(uint n) {
    Alignment newAlignment;
    newAlignment.pegNum = n;
    newAlignment.holeNum = n;

    vector<atomic_uint> A(n);
    vector<atomic_uint> invA(n);
    for (size_t i = 0; i < n; i++) {
        A[i].store(i, memory_order_relaxed);
        invA[i].store(i, memory_order_relaxed);
    }
    newAlignment.pegsToHoles = move(A);
    newAlignment.holesToPegs = move(invA);

    return newAlignment;
}

Alignment Alignment::reverse() const {
    Alignment newAlignment;
    newAlignment.pegNum = holeNum;
    newAlignment.holeNum = pegNum;

    vector<atomic_uint> newA(holeNum);
    vector<atomic_uint> newInvA(pegNum);

    for (size_t i = 0; i < pegNum; i++) {
        newInvA[i].store(pegsToHoles[i].load(memory_order_relaxed), memory_order_relaxed);
    }
    for (size_t i = 0; i < holeNum; i++) {
        newA[i].store(holesToPegs[i].load(memory_order_relaxed), memory_order_relaxed);
    }

    newAlignment.holesToPegs = move(newInvA);
    newAlignment.pegsToHoles = move(newA);

    return newAlignment;
}

Alignment Alignment::correctMapping(const Graph& G1, const Graph& G2) {
    if (not G1.hasSameNodeNamesAs(G2)) {
        throw runtime_error("cannot load correct mapping; nodes have different names");
    }
    vector<uint> A(G1.getNumNodes());
    for (uint i = 0; i < G1.getNumNodes(); i++) {
        A[i] = G2.getNameIndex(G1.getNodeName(i));
    }
    return Alignment(A, G2.getNumNodes());
}

vector<uint> Alignment::copyPegsToHoles() const {
    vector<uint> v;
    v.reserve(pegNum);
    for (const auto& e : pegsToHoles) {
        v.push_back(e.load(memory_order_relaxed));
    }
    return v;
}

vector<uint> Alignment::copyHolesToPegs() const {
    vector<uint> v;
    v.reserve(holeNum);
    for (const auto& e : holesToPegs) {
        v.push_back(e.load(memory_order_relaxed));
    }
    return v;
}

void Alignment::movePeg(uint peg, uint newHole) {
    uint oldHole = pegsToHoles[peg].exchange(newHole, memory_order_relaxed);
    holesToPegs[oldHole].store(invalidPeg, memory_order_relaxed);
    holesToPegs[newHole].store(peg, memory_order_relaxed);
}

void Alignment::movePeg(uint peg, uint oldHole, uint newHole) {
    pegsToHoles[peg].store(newHole, memory_order_relaxed);
    holesToPegs[oldHole].store(invalidPeg, memory_order_relaxed);
    holesToPegs[newHole].store(peg, memory_order_relaxed);
}

void Alignment::swapPegs(uint peg1, uint peg2) {
    uint hole1 = pegsToHoles[peg1].load(memory_order_relaxed);
    uint hole2 = pegsToHoles[peg2].exchange(hole1, memory_order_relaxed);
    pegsToHoles[peg1].store(hole2, memory_order_relaxed);

    holesToPegs[hole1].store(peg2, memory_order_relaxed);
    holesToPegs[hole2].store(peg1, memory_order_relaxed);
}

void Alignment::swapPegs(uint peg1, uint peg2, uint hole1, uint hole2) {
    pegsToHoles[peg1].store(hole2, memory_order_relaxed);
    pegsToHoles[peg2].store(hole1, memory_order_relaxed);

    holesToPegs[hole1].store(peg2, memory_order_relaxed);
    holesToPegs[hole2].store(peg1, memory_order_relaxed);
}

void Alignment::compose(const Alignment& other) {
    holeNum = other.holeNum;
    holesToPegs = move(vector<atomic_uint>(holeNum));
    for (auto &holesPeg: holesToPegs) holesPeg.store(invalidPeg, memory_order_relaxed);

    for (uint peg = 0; peg < numOfPegs(); peg++) {
        uint oldHole = pegsToHoles[peg].load(memory_order_relaxed);
        uint newHole = other.pegsToHoles[oldHole].load(memory_order_relaxed);
        pegsToHoles[peg].store(newHole, memory_order_relaxed);
        holesToPegs[newHole].store(peg, memory_order_relaxed);
    }
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
    for (const auto& edge: G1.getEdgeList()) {
        uint peg1 = edge[0];
        uint peg2 = edge[1];
        uint hole1 = pegsToHoles[peg1].load(memory_order_relaxed);
        uint hole2 = pegsToHoles[peg2].load(memory_order_relaxed);

        res += G2.getEdgeWeight(hole1, hole2);
    }
    return res;
}

bool Alignment::isCorrectlyDefined(const Graph& G1, const Graph& G2) const {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();
    if (n1 != pegNum || n2 != holeNum) {
        return false;
    }

    if (n1 != pegsToHoles.size() || n2 != holesToPegs.size()) {
        return false;
    }

    vector<uint> colorMap = G1.myColorIdsToOtherGraphColorIds(G2);

    for (uint peg = 0; peg < pegNum; ++peg) {
        uint hole = pegsToHoles[peg].load(memory_order_relaxed);
        if (hole >= invalidHole) {
            return false;
        }
        if (holesToPegs[hole].load(memory_order_relaxed) != peg) {
            return false;
        }

        uint pegColor = G1.getNodeColor(peg);
        uint holeColor = G2.getNodeColor(pegsToHoles[peg]);
        if (colorMap.at(pegColor) != holeColor) {
            return false;
        }
    }
    for (uint hole = 0; hole < holeNum; ++hole) {
        uint peg = holesToPegs[hole].load(memory_order_relaxed);
        if (peg > invalidPeg) {
            return false;
        }
        if (peg == invalidPeg) {
            continue;
        }
        if (pegsToHoles[peg].load(memory_order_relaxed) != hole) {
            return false;
        }
    }

    return true;
}

void Alignment::printDefinitionErrors(const Graph& G1, const Graph& G2) const {
    uint n1 = G1.getNumNodes();
    uint n2 = G2.getNumNodes();

    vector<uint> colorMap = G1.myColorIdsToOtherGraphColorIds(G2);

    int count = 0;

    if (pegNum != n1) {
        cerr << "Incorrect pegNum: "<<pegNum<<", should be "<<n1<<endl;
    }
    if (pegsToHoles.size() != n1) {
        cerr << "Incorrect pegsToHoles size: "<<pegsToHoles.size()<<", should be "<<n1<<endl;
    }
    if (holeNum != n2) {
        cerr << "Incorrect holeNum: "<<pegNum<<", should be "<<n2<<endl;
    }
    if (holesToPegs.size() != n2) {
        cerr << "Incorrect holesToPegs size: "<<holesToPegs.size()<<", should be "<<n2<<endl;
    }

    for (uint peg = 0; peg < pegNum; ++peg) {
        uint hole = pegsToHoles[peg].load(memory_order_relaxed);
        if (hole >= invalidHole) {
            cerr<<count++<<": peg "<<peg<<" ("<<G1.getNodeName(peg)<<") maps to hole ";
            cerr<<hole<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
        }
        uint peg2 = holesToPegs[hole].load(memory_order_relaxed);
        if (peg != peg2 && peg2 < invalidPeg) {
            cerr<<count++<<": peg "<<peg<<" ("<<G1.getNodeName(peg)<<") maps to hole "<<hole<<" (";
            cerr<<G2.getNodeName(hole)<<"), which actually maps back to peg "<<peg2;
            cerr<<" ("<<G1.getNodeName(peg2)<<")"<<endl;
        }
        uint pegColor = G1.getNodeColor(peg);
        uint holeColor = G2.getNodeColor(hole);
        if (colorMap.at(pegColor) != holeColor) {
            string pegColorName = G1.getColorName(pegColor);
            string holeColorName = G2.getColorName(holeColor);
            cerr<<count<<": peg "<<peg<<" ("<<G1.getNodeName(peg)<<") of color ";
            cerr<<G1.getColorName(pegColor)<<" maps to hole "<<pegsToHoles[peg]<<" (";
            cerr<<G2.getNodeName(pegsToHoles[peg])<<") of color "<<G2.getColorName(holeColor)<<endl;
            ++count;
        }
    }

    for (uint hole = 0; hole < pegNum; ++hole) {
        uint peg = holesToPegs[hole].load(memory_order_relaxed);

        if (peg > invalidPeg) {
            cerr<<count++<<": hole "<<hole<<" ("<<G2.getNodeName(hole)<<") maps to ";
            cerr<<peg<<", which is not neither a valid peg nor the None value ("<<invalidPeg<<")"<<endl;
        }
        if (peg == invalidPeg) {
            continue;
        }

        uint hole2 = pegsToHoles[peg].load(memory_order_relaxed);
        if (hole != hole2) {
            cerr<<count++<<": hole "<<hole<<" ("<<G2.getNodeName(hole)<<") maps to peg "<<peg<<" (";
            cerr<<G1.getNodeName(peg)<<"), which actually maps back to hole "<<hole2;
            cerr<<" ("<<G2.getNodeName(hole2)<<")"<<endl;
        }
    }
}

