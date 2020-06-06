#include "Alignment.hpp"
#include "Graph.hpp"
#include "utils/utils.hpp"
#include "./measures/MultiS3.hpp"

using namespace std;
extern bool _graphsSwitched;

Alignment::Alignment() {}

Alignment::Alignment(const vector<uint>& mapping): A(mapping) {}

Alignment::Alignment(const Alignment& alig): A(alig.A) {}

Alignment::Alignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList) {
    unordered_map<string,uint> mapG1 = G1->getNodeNameToIndexMap();
    unordered_map<string,uint> mapG2 = G2->getNodeNameToIndexMap();
    uint n1 = mapList.size();
    uint n2 = G2->getNumNodes();
    A = vector<uint>(G1->getNumNodes(), n2); //n2 used to note invalid value
    for (uint i = 0; i < n1; i++) {
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

Alignment Alignment::loadEdgeListUnordered(Graph* G1, Graph* G2, string fileName) {
    vector<string> edges = fileToStrings(fileName);
    vector<vector<string> > edgeList(edges.size()/2, vector<string> (2));
    vector<string> G1Names = G1->getNodeNames();
    for (uint i = 0; i < edges.size()/2; i++){ //G1 is always edgeList[i][0], not 1.
        if(find(G1Names.begin(), G1Names.end(), edges[2*i]) != G1Names.end()){ //if G1 contains the nodename edges[2*i].  If this is false, G2 must contain it and edges[2*i+1] must be in G1.
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
    unordered_map<string,uint> mapG1 = G1->getNodeNameToIndexMap();
    unordered_map<string,uint> mapG2 = G2->getNodeNameToIndexMap();
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    vector<uint> A(n1, n2); //n2 used to note invalid value
    for (uint i = 0; i < mapList.size(); i++) {
        string nodeG1 = mapList[i][0];
        string nodeG2 = mapList[i][1];

        if (byName) {
            bool nodeG1Misplaced = false;
            bool nodeG2Misplaced = false;
            if(mapG1.find(nodeG1) == mapG1.end()){
                cout << nodeG1 << " not in G1 " << G1->getName();
                if(mapG2.find(nodeG1) != mapG2.end()){
                    cout << ", but it is in G2. Will switch if appropriate." << endl;
                    nodeG1Misplaced = true;
                }else{
                    cout << endl;
                    continue;
                }
            }
            if (mapG2.find(nodeG2) == mapG2.end()){
                cout << nodeG2 << " not in G2 " << G2->getName();
                if(mapG1.find(nodeG2) != mapG1.end()){
                    //cout << " is in G1, though." << endl;
                    cout << ", but it is in G1. Will switch if appropriate." << endl;
                    nodeG2Misplaced = true;
                }else{
                    cout << endl;
                    continue;
                }
            }
            if(nodeG1Misplaced and nodeG2Misplaced){
                string temp = nodeG1;
                nodeG1 = nodeG2;
                nodeG2 = temp;
                cout << nodeG1 << " and " << nodeG2 << " swapped." << endl;
            }
            A[mapG1[nodeG1]] = mapG2[nodeG2];
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
    if (not G1.sameNodeNames(G2)) {
        throw runtime_error("cannot load correct mapping");
    }

    unordered_map<uint,string> G1Index2Name = G1.getIndexToNodeNameMap();
    unordered_map<string,uint> G2Name2Index = G2.getNodeNameToIndexMap();

    uint n = G1.getNumNodes();
    vector<uint> A(n);
    for (uint i = 0; i < n; i++) {
        A[i] = G2Name2Index[G1Index2Name[i]];
    }
    return Alignment(A);
}

vector<uint> Alignment::getMapping() const {
    return A;
}

uint& Alignment::operator[] (uint node) {
    return A[node];
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

const uint& Alignment::operator[](const uint node) const {
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

typedef unordered_map<uint,string> NodeIndexMap;
void Alignment::writeEdgeList(Graph const * G1, Graph const * G2, ostream& edgeListStream) const {
    NodeIndexMap mapG1 = G1->getIndexToNodeNameMap();
    NodeIndexMap mapG2 = G2->getIndexToNodeNameMap();
    for (uint i = 0; i < size(); ++i){
        if (_graphsSwitched)
            edgeListStream << mapG2[A[i]] << "\t" << mapG1[i] << endl;
        else
            edgeListStream << mapG1[i] << "\t" << mapG2[A[i]] << endl;
        }
}


uint Alignment::numAlignedEdges(const Graph& G1, const Graph& G2) const {
    vector<vector<uint> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    Matrix<MATRIX_UNIT> G2Matrix;
    G2.getMatrix(G2Matrix);

    uint count = 0;
    for (const auto& edge: G1EdgeList) {
        uint node1 = edge[0], node2 = edge[1];
        count += G2Matrix[A[node1]][A[node2]];
    }
    return count;
}

int Alignment::numSquaredAlignedEdges(const Graph& G1, const Graph& G2) const {
    vector<vector<uint> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    Matrix<MATRIX_UNIT> G2Matrix;
    G2.getMatrix(G2Matrix);

#if 0
    Pseudo-code (assuming you have initially removed g1 from g2)
    for every entry (i,j) in the lower triangle of the G2 adjacency matrix
        rungs = G2.adjMatrix(i,j)
        if there is an edge between the *pegs* in the hole (i,j), then rungs++
        count += rungs * rungs;
    end for
#endif


#ifdef MULTI_PAIRWISE
    // Before computing rung sizes, we need to add the edges
    // from G1 that we pruned back to G2
    for (const auto& edge: G1EdgeList) {
        uint hole1 = A[edge[0]];
        uint hole2 = A[edge[1]];
        G2Matrix[hole1][hole2] += 1;
	G2Matrix[hole2][hole1] += 1;
    }
#endif

    int count = 0;
    uint n2 = G2.getNumNodes(); 
    for(uint i = 0; i < n2; i++){
        for(uint j = 0; j < i; j++){
            int rungs  = G2Matrix[i][j];
            count += rungs * rungs;
        }
    }
    assert(count > 0); // guard against overflow
    return count;
}

#if MULTI_PAIRWISE
int Alignment::numExposedEdges(const Graph& G1, const Graph& G2) const {
    int ret = 0;
    const uint n1 = G1.getNumNodes();
    const uint n2 = G2.getNumNodes();

    Matrix<MATRIX_UNIT> G1Matrix;
    Matrix<MATRIX_UNIT> G2Matrix;
    G1.getMatrix(G1Matrix);
    G2.getMatrix(G2Matrix);
    
    vector<vector<uint>> G2EdgeList;
    G2.getEdgeList(G2EdgeList);
    
	
	vector<uint> whichPeg(n2, n1); // value of n1 represents not used
	for (uint i = 0; i < n1; ++i)
	{
		//assert(0 <= A[i] && A[i] < n2);
		whichPeg[A[i]] = i; // inverse of the alignment
	}
        
	for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j)
        {
                bool exposed = (G2Matrix[i][j] > 0);
		if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1)
		{
			if(G1Matrix[whichPeg[i]][whichPeg[j]])
			{
				//assert(G1Matrix[whichPeg[j]][whichPeg[i]]);
				exposed = true;
			} else
				; // assert(!G1Matrix[whichPeg[j]][whichPeg[i]]);
		}
		if(exposed) ret++;
	}
        //cout << "G2------------" << count << endl;
		
#if 0
	unordered_set<uint> holes(n2);
	unordered_set<uint>::iterator iter;
	bool hole1 = false;
	bool hole2 = false;
	
    for (uint i = 0; i < n2; ++i)
    {
		iter = find(holes.begin(), holes.end(), i);
		if (iter != holes.end())
		{
			hole1 = true; // hole1 exposure already counted
		}
		
		for(uint j = 0; j < i; ++j)
		{
			iter = find(holes.begin(), holes.end(), j);
			if (iter != holes.end())
			{
				hole2 = true; // hole2 exposure already counted
			}
			
			if (G2Matrix[i][j] && (hole1 && !hole2)) // hole1 counted, but hole2 not yet counted
			{
				++ret;
				holes.insert(j);
			}
			else if (G2Matrix[i][j] && (!hole1 && hole2)) // hole2 counted, but hole1 not yet counted
			{
				++ret;
				holes.insert(i);
				hole1 = true;
			}
			else if (G2Matrix[i][j] && (!hole1 && !hole2)) // neither hole counted
			{
				ret += 2;
				holes.insert(i);
				holes.insert(j);
				hole1 = true;
			}
		}
    }
#endif
    return ret;
}

unsigned Alignment::multiS3Numerator(const Graph& G1, const Graph& G2) const {
     
    uint ret = 0;
    vector<vector<uint> > G1EdgeList;
    vector<vector<uint> > G2EdgeList;
    G1.getEdgeList(G1EdgeList);
    G2.getEdgeList(G2EdgeList);
    Matrix<MATRIX_UNIT> G1Matrix;
    Matrix<MATRIX_UNIT> G2Matrix;
    G1.getMatrix(G1Matrix);
    G2.getMatrix(G2Matrix);
    uint node1, node2;
        
	switch (MultiS3::numerator_type){
        case 1:
        case 2:
        {
            for (const auto& edge: G1EdgeList)
            {
                node1 = edge[0], node2 = edge[1];
                if(MultiS3::numerator_type == MultiS3::ra_i){
                    // numerator is ra_i
                    if   (G2Matrix[A[node1]][A[node2]]>=1){
                        ret += G2Matrix[A[node1]][A[node2]] + 1; // +1 because G1 was pruned out of G2
                    }
                }else if (MultiS3::numerator_type == MultiS3::la_i){
                    if (G2Matrix[A[node1]][A[node2]]>=1){
                        ret += 1;                                // +1 because it is a ladder
                    }
                //}else if (MultiS3::numerator_type == MultiS3::ra_global){
                 //}else if (MultiS3::numerator_type == MultiS3::la_global){}
                }else if (MultiS3::numerator_type == MultiS3::_default){//default
                    ret += G2Matrix[A[node1]][A[node2]] + 1;
                }
            }
        }
            break;
        case 4:
        case 3:
        {
            const uint n1 = G1.getNumNodes();
            const uint n2 = G2.getNumNodes();
            vector<uint> reverse_A = vector<uint> (n2,n1);// value of n1 represents not used
            for(uint i=0; i< n1; i++){
                reverse_A[A[i]] = i;
            }

            for (const auto& edge: G2EdgeList){
                node1 = edge[0], node2 = edge[1];
                if (MultiS3::numerator_type == MultiS3::ra_global){
                    ret += G2Matrix[node1][node2]>1 ? G2Matrix[node1][node2]:0;
                    if (reverse_A[node1] < n1 and reverse_A[node2] < n1 and G2Matrix[node1][node2]>0 and G1Matrix[reverse_A[node1]][reverse_A[node2]]==1){
                        ret += G2Matrix[node1][node2]>1 ? 1:2;
                        // +1 because G1 was pruned out of G2
                    }
                }else if (MultiS3::numerator_type == MultiS3::la_global){
                    if ((G2Matrix[node1][node2]>1) or (reverse_A[node1] < n1 and reverse_A[node2] < n1 and G2Matrix[node1][node2] + G1Matrix[reverse_A[node1]][reverse_A[node2]] > 1)){
                        ret += 1; // +1 because G1 was pruned out of G2
                    }
                }
            }
        }
            break;
        default:
        {
            for (const auto& edge: G1EdgeList)
            {
               node1 = edge[0], node2 = edge[1];
               ret += G2Matrix[A[node1]][A[node2]] + 1; // +1 because G1 was pruned out of G2
               //cerr << "numerator_type not specified, Using default numerator." << endl;
             }
        }
            break;
	}
    return ret;
    
}

unsigned Alignment::multiS3Denominator(const Graph& G1, const Graph& G2) const {
    uint ret = 0;
    vector<vector<uint> > G1EdgeList;
    vector<vector<uint> > G2EdgeList;
    G1.getEdgeList(G1EdgeList);
    G2.getEdgeList(G2EdgeList);
    Matrix<MATRIX_UNIT> G1Matrix;
    Matrix<MATRIX_UNIT> G2Matrix;
    G1.getMatrix(G1Matrix);
    G2.getMatrix(G2Matrix);
    uint node1, node2;
    const uint n1 = G1.getNumNodes();
    const uint n2 = G2.getNumNodes();
    vector<uint> whichPeg(n2, n1); // value of n1 represents not used

    for (uint i = 0; i < n1; ++i){
        whichPeg[A[i]] = i; // inverse of the alignment
    }
    
    switch (MultiS3::denominator_type){
        case 3:
        case 4:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (G2Matrix[i][j] > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G1Matrix[whichPeg[i]][whichPeg[j]] > 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::ee_global) ret++;
                if(exposed and MultiS3::denominator_type == MultiS3::rt_global){
                    if (whichPeg[i] < n1 && whichPeg[j] < n1){
                        ret += G1Matrix[whichPeg[i]][whichPeg[j]] + G2Matrix[i][j];
                    }else{
                        ret +=  G2Matrix[i][j];
                    }
                }
            }
        }
            break;
        case 2:
        case 1:
        {
            for (uint i = 0; i < n2; ++i) for (uint j = 0; j < i; ++j){
                bool exposed = (whichPeg[i] < n1 && whichPeg[j] < n1 && G1Matrix[whichPeg[i]][whichPeg[j]] > 0);
                if(!exposed && whichPeg[i] < n1 && whichPeg[j] < n1 && G2Matrix[i][j]> 0){
                    exposed = true;
                }
                if(exposed and MultiS3::denominator_type == MultiS3::ee_i) ret++;
                if(exposed and MultiS3::denominator_type == MultiS3::rt_i){
                    if(whichPeg[i] < n1 && whichPeg[j] < n1){
                        ret += G1Matrix[whichPeg[i]][whichPeg[j]] + G2Matrix[i][j];
                    }
                    else{
                        ret += G2Matrix[i][j];
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif

Graph Alignment::commonSubgraph(const Graph& G1, const Graph& G2) const {
    uint n = G1.getNumNodes();

    vector<vector<uint> > G1EdgeList;
    G1.getEdgeList(G1EdgeList);
    Matrix<MATRIX_UNIT> G2Matrix;

    G2.getMatrix(G2Matrix);

    //only add edges preserved by alignment
    vector<vector<uint> > edgeList(0);
    for (const auto& edge: G1EdgeList) {
        uint node1 = edge[0], node2 = edge[1];
        if (G2Matrix[A[node1]][A[node2]]) {
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
    unordered_map<uint,string> G1Names = G1.getIndexToNodeNameMap();
    unordered_map<uint,string> G2Names = G2.getIndexToNodeNameMap();

    vector<bool> G2AssignedNodes(n2, false);
    int count = 0;
    if (A.size() != n1) 
        cerr << "Incorrect size: "<<A.size()<<", should be "<<n1<<endl;
    for (uint i = 0; i < n1; i++) {
        if (A[i] < 0 or A[i] >= n2) {
            cerr<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<", which is not in range 0..n2 ("<<n2<<")"<<endl;
            count++;
        }
        if (G2AssignedNodes[A[i]]) {
            cerr<<count<<": node "<<i<<" ("<<G1Names[i]<<") maps to "<<A[i]<<" ("<<G2Names[A[i]]<<"), which is also mapped to by a previous node"<<endl;
            count++;
        }
        G2AssignedNodes[A[i]] = true;
    }
}

Alignment Alignment::randomAlignmentWithLocking(Graph* G1, Graph* G2){
    assert(G1->getLockedCount() == G2->getLockedCount());

    unordered_map<uint,string> g1_NameMap = G1->getIndexToNodeNameMap();
    unordered_map<string,uint> g2_IndexMap = G2->getNodeNameToIndexMap();
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();

    vector<uint> A(n1, n2); //n2 used to note invalid value
    for (uint i = 0; i < n1; i++) {
        if(!G1->isLocked(i))
            continue;
        string node1 = g1_NameMap[i];
        string node2 = G1->getLockedTo(i);
        uint node2Index = g2_IndexMap[node2];
        A[i] = node2Index;
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

Alignment Alignment::startingMultipartiteAlignment(Graph* G1, Graph* G2) {
    unordered_map<uint,string> g1_NameMap = G1->getIndexToNodeNameMap();
    unordered_map<string,uint> g2_IndexMap = G2->getNodeNameToIndexMap();
    
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    
    vector<vector<uint>> G2UnlockedTypedIndices(G2->typedNodesIndexList.size(), vector<uint>{});
    for(uint i = 0; i < n2; i++){
        if(G2->isLocked(i))
            continue;
        G2UnlockedTypedIndices[G2->nodeTypes[i]].push_back(i);
    }
    
    vector<uint> A(n1, n2);
    
    for (uint i = 0; i < n1; i++) {
        if(!G1->isLocked(i))
            continue;
        string node1 = g1_NameMap[i];
        string node2 = G1->getLockedTo(i);
        uint node2Index = g2_IndexMap[node2];
        if(G1->nodeTypes[i] != G2-> nodeTypes[node2Index]){
            cout << "Invalid lock -- cannot lock two nodes of different types " << node1 <<  ", " << node2 << endl;
        }
        assert (G1->nodeTypes[i] == G2-> nodeTypes[node2Index]);
        A[i] = node2Index;
    }
    
    
    vector<bool> G2AssignedNodes(n2, false);
    for (uint i = 0; i < n1; i++) {
        if (A[i] != n2)
            G2AssignedNodes[A[i]] = true;
    }
    
    for (uint i = 0; i < n1; i++) {
        if (A[i] == n2) {
            int type = G1->nodeTypes[i];
            int randSize = G2UnlockedTypedIndices[type].size();
            int j = randMod(randSize);
            while (G2AssignedNodes[G2UnlockedTypedIndices[type][j]])
                j = randMod(randSize);
            A[i] = G2UnlockedTypedIndices[type][j];
            G2AssignedNodes[G2UnlockedTypedIndices[type][j]] = true;
        }
    }
    
    for (uint i = 0; i < n1; i++) {
        assert (G1->nodeTypes[i] == G2-> nodeTypes[A[i]]);
    }

    Alignment alig(A);
    alig.printDefinitionErrors(*G1, *G2);
    assert(alig.isCorrectlyDefined(*G1, *G2));
    return alig;
}

Alignment Alignment::randomAlignmentWithNodeType(Graph* G1, Graph* G2){
        assert(G1->getLockedCount() == G2->getLockedCount());

        unordered_map<uint,string> g1_NameMap = G1->getIndexToNodeNameMap();
        unordered_map<string,uint> g2_IndexMap = G2->getNodeNameToIndexMap();
        uint n1 = G1->getNumNodes();
        uint n2 = G2->getNumNodes();

        vector<uint> G2_UnlockedGeneIndexes;
        vector<uint> G2_UnlockedRNAIndexes;
        for(uint i=0;i<n2;i++){
            if(G2->isLocked(i))
                continue;
            if(G2->nodeTypes[i] == Graph::NODE_TYPE_GENE) //  "gene")
                G2_UnlockedGeneIndexes.push_back(i);
            else if(G2->nodeTypes[i] == Graph::NODE_TYPE_MIRNA) // "miRNA")
                G2_UnlockedRNAIndexes.push_back(i);
        }

        vector<uint> A(n1, n2); //n2 used to note invalid value
        for (uint i = 0; i < n1; i++) {
            // Aligns all the locked nodes together
            if(!G1->isLocked(i))
                continue;
            string node1 = g1_NameMap[i];
            string node2 = G1->getLockedTo(i);
            uint node2Index = g2_IndexMap[node2];
            if(G1->nodeTypes[i] != G2-> nodeTypes[node2Index]){
                cout << "Invalid lock -- cannot lock a gene to a miRNA " << node1 <<  ", " << node2 << endl;
            }
            assert (G1->nodeTypes[i] == G2-> nodeTypes[node2Index]);
            A[i] = node2Index;
        }
        vector<bool> G2AssignedNodes(n2, false);
        for (uint i = 0; i < n1; i++) {
            if (A[i] != n2)
                G2AssignedNodes[A[i]] = true;
        }

        for (uint i = 0; i < n1; i++) {
            if (A[i] == n2) {
                if(G1->nodeTypes[i] == Graph::NODE_TYPE_GENE){ // "gene"){
                    int randSize = G2_UnlockedGeneIndexes.size();
                    int j = randMod(randSize);
                    while (G2AssignedNodes[G2_UnlockedGeneIndexes[j]])
                        j = randMod(randSize);
                    A[i] = G2_UnlockedGeneIndexes[j];
                    G2AssignedNodes[G2_UnlockedGeneIndexes[j]] = true;
                }
                else if(G1->nodeTypes[i] == Graph::NODE_TYPE_MIRNA){// "miRNA"){
                    int randSize = G2_UnlockedRNAIndexes.size();
                    int j = randMod(randSize);
                    while (G2AssignedNodes[G2_UnlockedRNAIndexes[j]])
                        j = randMod(randSize);
                    A[i] = G2_UnlockedRNAIndexes[j];
                    G2AssignedNodes[G2_UnlockedRNAIndexes[j]] = true;
                }
            }
        }

        for (uint i = 0; i < n1; i++) {
            assert (G1->nodeTypes[i] == G2-> nodeTypes[A[i]]);
        }

        Alignment alig(A);
        alig.printDefinitionErrors(*G1, *G2);
        assert(alig.isCorrectlyDefined(*G1, *G2));
        return alig;
}




void Alignment::reIndexBefore_Iterations(unordered_map<uint, uint> reIndexMap){
    vector<uint> resA = vector<uint> (A.size());
    for(uint i=0; i< A.size();i++){
        uint b = reIndexMap[i];
        resA[b] = A[i];
    }
    A = resA;
}

void Alignment::reIndexAfter_Iterations(unordered_map<uint, uint> reIndexMap){
    vector<uint> resA = vector<uint> (A.size());
    for(uint i=0; i< A.size();i++){
        resA[i] = A[reIndexMap[i]];
    }
    A = resA;
}
