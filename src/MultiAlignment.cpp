#include "MultiAlignment.hpp"
#include "Graph.hpp"
//#include "Shadownetwork.hpp"
#include "utils/utils.hpp"
using namespace std;

MultiAlignment::MultiAlignment(const vector<vector<ushort> >& mapping,ushort shadowNetworkSize): MA(mapping)
{
    ushort emptyValue = 0-1;
	upAlign = vector<vector<ushort>>(0);
    vector<ushort> temp;
	for (uint i = 0; i < mapping.size(); i++) {
		vector<ushort> temp = vector<ushort>(shadowNetworkSize,emptyValue);
		for(uint j = 0; j < mapping[i].size(); j++) {
			temp[mapping[i][j]] = j;
		}
		upAlign.push_back(temp);
	}
}

MultiAlignment::MultiAlignment(const MultiAlignment& alig): MA(alig.MA),upAlign(alig.upAlign){}


/*
    MultiAlignment::MultiAlignment(const vector<Graph>* GV, Graph* SN, const vector<vector<string> >& mapList) {
    map<string,ushort> mapSN = SN->getNodeNameToIndexMap();
    for (Graph& G: GV)
    {
        map<string,ushort> mapG = G->getNodeNameToIndexMap();
        ushort n1 = mapList.size();
        ushort n2 = SN->getNumNodes();
        A = vector<ushort>(G->getNumNodes(), n2); //n2 used to note invalid value
        for (ushort i = 0; i < n1; i++) {
            string nodeG = mapList[i][0];
            string nodeSN = mapList[i][1];
            A[i][mapG[nodeG]] = mapSN[nodeSN];
        }
    printDefinitionErrors(*G,*SN);
    assert(isCorrectlyDefined(*G, *SN));
    }
}*/

/*MultiAlignment::MultiAlignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList) {
	need to change later
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
}*/

/*MultiAlignment MultiAlignment::loadMapping(string fileName) {
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
}*/

MultiAlignment MultiAlignment::random(vector<ushort> netWorkSizes, ushort shadowNetworkSize) {
	//taken from: http://stackoverflow.com/questions/311703/algorithm-for-sampling-without-replacement
	cerr <<"check at  the beginning of MMultiAlignment MultiAlignment::random(vector<ushort> netWorkSizes, ushort shadowNetworkSize) @ MultiAlignment.cpp"<<endl;
	vector<vector<ushort> > alignment= vector<vector<ushort> >(netWorkSizes.size());
	for (uint i = 0; i < netWorkSizes.size(); i++) {
		alignment[i] = vector<ushort>(netWorkSizes[i]);
	    ushort t = 0; // total input records dealt with
	    ushort m = 0; // number of items selected so far
	    double u;
	    while (m < netWorkSizes[i]) {
		    u = randDouble();
		    if ((shadowNetworkSize - t)*u >= netWorkSizes[i] - m) {
			    t++;
		    }
		    else {
			    alignment[i][m] = t;
			    t++;
			    m++;
		    }
	    }
	    randomShuffle(alignment[i]);
	}
	cerr <<"check after randomShuffle @ MultiAlignment.cpp"<<endl;
    return MultiAlignment(alignment,shadowNetworkSize);

}

MultiAlignment MultiAlignment::empty() {
	vector<vector<ushort> > emptyAlignment = vector<vector<ushort>>(0);
	return MultiAlignment(emptyAlignment,0);
}
/*
MultiAlignment MultiAlignment::identity(uint n) {
	vector<vector<ushort>> A(n);
	for (uint i = 0; i < n; i++) {
		A[i] = i;
	}
	return Alignment(A);
}*/
/*
MultiAlignment MultiAlignment::correctMapping(const Graph& G1, const Graph& G2) {
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
*/
vector<vector<ushort> > MultiAlignment::getMapping() const {
	return MA;
}

vector<vector<ushort> > MultiAlignment::getUpAlign() const {
	return upAlign;
}

vector<ushort>& MultiAlignment::operator[] (ushort node) {
	return MA[node];
}

const vector<ushort>& MultiAlignment::operator[](const ushort node) const {
	return MA[node];
}

uint MultiAlignment::size() const {
	return MA.size();
}

void MultiAlignment::write(ostream& stream) const {

	uint column = size();
	uint row = upAlign[0].size();
	ushort emptyValue = 0-1;
	for (uint i = 0; i < row; i++) {
		for (uint j = 0; j < column; j++) {
			if(upAlign[j][i]!=emptyValue){
		        stream << upAlign[j][i] << "\t";
		    }else{
				stream <<"EMPTY"<< "\t";
			}
        }
        stream << endl;
	}
	stream << endl;
}

void MultiAlignment::writeEdgeList(const vector<Graph>& GV, const Graph& SN, ostream& stream) const {
	map<ushort,string> mapSN = SN.getIndexToNodeNameMap();
	vector<map<ushort,string>> maps = vector<map<ushort,string>>(0);
	ushort emptyValue = 0-1;
	stream <<SN.getName()<< "\t";
	for (uint i = 0; i < GV.size(); i++){
		stream <<GV[i].getName()<< "\t";
    }
    stream << endl;
	for (uint i = 0; i < GV.size(); i++){
        maps.push_back(GV[i].getIndexToNodeNameMap());
    }
	for (uint i = 0; i < upAlign[0].size(); i++) {
		stream <<mapSN[i]<< "\t";
		for (uint j = 0; j < GV.size(); j++) {
			if(upAlign[j][i]!=emptyValue){
			    stream <<maps[j][upAlign[j][i]]<< "\t";
			}
			else{
				stream <<"EMPTY"<< "\t";
			}
		}
		stream << endl;
	}
	stream<<endl;
}


/*uint MultiAlignment::numAlignedEdges(const Graph& G1, const Graph& G2) const {
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
*/
/*
uint MultiAlignment::weight(vector<Graph>* GV, Graph* SN, ushort n1, ushort n2) const {
    uint count = 0;
    vector<vector<bool> > AdjMatrix;
    //cerr<<"check at the begining of MultiAlignment::weight @ MultiAlignment.cpp"<<endl;
    //cerr<<"A.size() = "<<A.size()<<" (*GV).size() = "<<(*GV).size()<<endl;
    //cerr<<"(*GV)[0].getNumNodes() = "<<(*GV)[0].getNumNodes()<<" (*GV)[1].getNumNodes() = "<<(*GV)[1].getNumNodes()<<endl;
    ushort empty = 0-1;
    SN->getAdjMatrix(AdjMatrix);
    count+=AdjMatrix[n1][n2];
    for (uint i = 0; i < A.size(); i++) {
		if((A[i][n1]!=empty)&&(A[i][n2]!=empty)){
		    (*GV)[i].getAdjMatrix(AdjMatrix);
		    count+=AdjMatrix[A[i][n1]][A[i][n2]];
		}
	}
	//cerr<<"check at the end of MultiAlignment::weight @ MultiAlignment.cpp"<<endl;
	return count;
}
*/
/*
Graph MultiAlignment::commonSubgraph(const Graph& G1, const Graph& G2) const {
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
*/
/*
void MultiAlignment::compose(const Alignment& other) {
	for (uint i = 0; i < size(); i++) {
		A[i] = other.A[A[i]];
	}
}
*/
bool MultiAlignment::isCorrectlyDefined(const vector<Graph>& GV, const Graph& SN) {
	uint SNsize = SN.getNumNodes();

	if(GV.size() != upAlign.size()) return false;
	for (uint i = 0; i < GV.size(); i++) {
		if (upAlign[i].size() != SNsize) return false;
		if (MA[i].size() != GV[i].getNumNodes()) return false;
	}
	return true;
}
/*
void MultiAlignment::printDefinitionErrors(const Graph& G1, const Graph& G2) {
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
*/
/*
MultiAlignment MultiAlignment::randomAlignmentWithLocking(Graph* G1, Graph* G2){
	assert(G1->getLockedCount() == G2->getLockedCount());

	map<ushort,string> g1_NameMap = G1->getIndexToNodeNameMap();
	map<string,ushort> g2_IndexMap = G2->getNodeNameToIndexMap();
	uint n1 = G1->getNumNodes();
	uint n2 = G2->getNumNodes();

	vector<ushort> A(n1, n2); //n2 used to note invalid value
	for (ushort i = 0; i < n1; i++) {
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
*/
/*
MultiAlignment MultiAlignment::randomAlignmentWithNodeType(Graph* G1, Graph* G2){
        assert(G1->getLockedCount() == G2->getLockedCount());

        map<ushort,string> g1_NameMap = G1->getIndexToNodeNameMap();
        map<string,ushort> g2_IndexMap = G2->getNodeNameToIndexMap();
        uint n1 = G1->getNumNodes();
        uint n2 = G2->getNumNodes();

        vector<ushort> G2_UnlockedGeneIndexes;
        vector<ushort> G2_UnlockedRNAIndexes;
        for(ushort i=0;i<n2;i++){
            if(G2->isLocked(i))
                continue;
            if(G2->nodeTypes[i] == "gene")
                G2_UnlockedGeneIndexes.push_back(i);
            else if(G2->nodeTypes[i] == "miRNA")
                G2_UnlockedRNAIndexes.push_back(i);
        }

        vector<ushort> A(n1, n2); //n2 used to note invalid value
        for (ushort i = 0; i < n1; i++) {
            // Aligns all the locked nodes together
            if(!G1->isLocked(i))
                continue;
            string node1 = g1_NameMap[i];
            string node2 = G1->getLockedTo(i);
            uint node2Index = g2_IndexMap[node2];
            if(G1->nodeTypes[i] != G2-> nodeTypes[node2Index]){
                cerr << "Invalid lock -- cannot lock a gene to a miRNA " << node1 <<  ", " << node2 << endl;
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
                if(G1->nodeTypes[i] == "gene"){
                    int randSize = G2_UnlockedGeneIndexes.size();
                    int j = randMod(randSize);
                    while (G2AssignedNodes[G2_UnlockedGeneIndexes[j]])
                        j = randMod(randSize);
                    A[i] = G2_UnlockedGeneIndexes[j];
                    G2AssignedNodes[G2_UnlockedGeneIndexes[j]] = true;
                }
                else if(G1->nodeTypes[i] == "miRNA"){
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
*/


/*
void MultiAlignment::reIndexBefore_Iterations(map<ushort, ushort> reIndexMap){
	vector<ushort> resA = vector<ushort> (A.size());
	for(uint i=0; i< A.size();i++){
		ushort b = reIndexMap[i];
		resA[b] = A[i];
	}
	A = resA;
}

void MultiAlignment::reIndexAfter_Iterations(map<ushort, ushort> reIndexMap){
	vector<ushort> resA = vector<ushort> (A.size());
	for(uint i=0; i< A.size();i++){
		resA[i] = A[reIndexMap[i]];
	}
	A = resA;
}
*/