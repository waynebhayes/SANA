#include "Database.hpp"

using namespace std;

Database::Database()
	: Database(5, 3, 500)
{}

Database::Database(uint k, uint radius, uint limit)
	: k_(k)
	, radius_(radius)
	, limit_(limit)
{
	uint m, i, j;
	string str;
	ifstream fcanon_map("data/canon_map"+to_string(k)+".txt"), forbit_map("data/orbit_map"+to_string(k)+".txt");
	ifstream fcanon_list("data/canon_list"+to_string(k)+".txt");

	//reading canon_map
	while(fcanon_map >> i >> str){
		canonDec_.push_back(i);
		canonPerm_.push_back(str);
	}
	fcanon_map.close();

	//reading canon_list
	fcanon_list >> m; //reading the number of canonical graphettes
	while(fcanon_list >> i){
		canonList_.push_back(i);
	}
	fcanon_list.close();

	//reading orbit_map
	forbit_map >> numOrbitId_; //reading the number of orbit ids
	for(i = 0; i < canonList_.size(); i++){
		vector<uint> inputvec;
		for(j = 0; j < k; j++){
			forbit_map >> m;
			inputvec.push_back(m);
		}
		orbitId_.push_back(inputvec);
	}
	forbit_map.close();
}

void Database::addGraph(string filename){
	ifstream fgraph(filename);
	vector<pair<uint, uint>> edgelist;
	uint m, n, i, j;
	int pos, len;
	string graphName(filename);
	while((pos=graphName.find("/"))>0) {
	    len = graphName.size();
	    graphName = graphName.substr(pos,len);
	}

	//reading edgelist
	while(fgraph >> m >> n){
		edgelist.push_back(make_pair(m, n));
	}
	fgraph.close();

	Graph graph(edgelist);
	vector<vector<bool>> sigMatrix(graph.numNodes(), vector<bool>(numOrbitId_, 0));//each row for each node
	vector<bool> haveOpen(numOrbitId_, 0);
	vector<ofstream> forbitId(numOrbitId_);

	for(i = 0; i < limit_; i++){
		Graphette* g = graph.sampleGraphette(k_, radius_);
		uint gDec = g->decimalNumber();
		//index of canonical isomorph of g in canonList
		uint cgIndex = this->getIndex(canonList_, canonDec_[gDec]);
		for(j = 0; j < k_; j++){
			uint oj = canonPerm_[gDec].at(j)-'0'; //original index of jth element of the canonical isomorph 
			uint id = orbitId_[cgIndex][j];
			if(!haveOpen[id]) {
			    forbitId[id].open("test/"+to_string(id)+"/"+graphName, ios_base::app);
			    haveOpen[id]=1;
			}
			forbitId[id] << (g->label(oj)) << " ";
			for(auto orbit: g->labels()){
				if(orbit != g->label(oj))
					forbitId[id] << orbit << " ";
			}
			forbitId[id] << endl;
			sigMatrix[g->label(oj)][id]= true;
		}
		delete g;
	}
	for(i=0; i < numOrbitId_; i++) if(haveOpen[i]) forbitId[i].close();
	ofstream fout("test/output");
	fout << k_ << " " << radius_ << " " << limit_ << endl;
	for(auto a: sigMatrix){
		for(auto b: a) fout << b;
		fout << endl;
	}
	fout.close();
}

uint Database::getIndex(vector<uint>& vec, uint num){
	uint index = lower_bound(vec.begin(), vec.end(), num)-vec.begin();
	if(vec[index] != num){
		throw domain_error("Database::getIndex(): num not found in vec");
	}
	else {
		return index;
	}
}

bool Database::queryGraphette(uint decimal){
	Graphette g(k_, decimal);
	uint j;
	uint cgIndex = this->getIndex(canonList_, canonDec_[decimal]);
	vector<uint> sig(k_);
	for(j = 0; j < k_; j++){
		uint oj = canonPerm_[decimal].at(j)-'0'; //original index of jth element of the canonical isomorph 
		sig[oj] = orbitId_[cgIndex][j];
	}
	return false;
}
