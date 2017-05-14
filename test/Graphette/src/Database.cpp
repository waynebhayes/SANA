#include "Database.hpp"

using namespace std;

Database::Database()
	: Database(5)
{}

Database::Database(ullint k)
	: k_(k)
{
	/**Just reading the data from canon_map, canon_list, and orbit_map**/
	ullint m, decimal;
	string permutation;
	ifstream fcanon_map("data/canon_map"+to_string(k)+".txt"), forbit_map("data/orbit_map"+to_string(k)+".txt");
	ifstream fcanon_list("data/canon_list"+to_string(k)+".txt");
	databaseDir = "Database"+to_string(k)+"/";

	//reading canon_map
	graphette x;
	while(fcanon_map >> decimal >> permutation){
		x.canonicalDecimal = decimal;
		x.canonicalPermutation = permutation;
		g.push_back(x);
	}
	fcanon_map.close();
	//reading canon_list and orbit_map
	fcanon_list >> m; //reading the number of canonical graphettes
	forbit_map >> numOrbitId_; //reading the number of orbit ids
	for(ullint i = 0; i < m; i++){
		fcanon_list >> decimal;
		canonicalGraphette.push_back(decimal);
		//reading orbit ids for the canonical graphette decimal
		vector<ullint> ids(k);
		for(ullint j = 0; j < k; j++){
			forbit_map >> ids[j];
		}
		orbitId_.push_back(ids);
	}
	fcanon_list.close();
	fcanon_map.close();
	forbit_map.close();
}

void Database::addGraph(string filename, ullint numSamples){
	//Prepocessing the input filename
	ifstream fgraph(filename);
	string graphName = filename;
	while(graphName.find("/") != string::npos)
		graphName = graphName.substr(graphName.find("/") + 1);
	//reading the graph from an edge list file
	vector<pair<ullint, ullint>> edgelist;
	ullint m, n;
	while(fgraph >> m >> n){
		edgelist.push_back(make_pair(m, n));
	}
	fgraph.close();
	Graph graph(edgelist);
	//orbit signature for each node
	vector<vector<bool>> orbitSignature(graph.numNodes(), vector<bool>(numOrbitId_, false));//each row for each node
	vector<bool> isOpen(numOrbitId_, false);
	vector<ofstream> forbitId(numOrbitId_);
	ullint indicator = numSamples / 10;
	for(ullint i = 0; i < numSamples; i++){
		if(i % indicator == 0)
			cout << (i / indicator) * 10 << "% complete\n";
		auto edge = xrand(0, edgelist.size());
		Graphette* x = graph.sampleGraphette(k_, edgelist[edge].first, edgelist[edge].second);
		Graphette* y = this->getCanonicalGraphette(x);
		delete x;
		//save orbit ids in respective directories
		for(ullint j = 0; j < k_; j++){
			auto z = y->decimalNumber();
			ullint l = lower_bound(canonicalGraphette.begin(), canonicalGraphette.end(), z) - canonicalGraphette.begin();
			ullint id = orbitId_[l][j];
			if(!isOpen[id]) {
			    forbitId[id].open(databaseDir+to_string(id)+"/"+graphName, ios_base::app);
			    isOpen[id] = true;
			}
			forbitId[id] << y->label(j) << " ";
			for(auto orbit: y->labels()){
				if(orbit != y->label(j))
					forbitId[id] << orbit << " ";
			}
			forbitId[id] << endl;
			orbitSignature[y->label(j)][id]= true;
		}
		delete y;
	}
	cout << "100% complete\n";
	for(ullint i = 0; i < numOrbitId_; i++) 
		if(isOpen[i]) forbitId[i].close();
	
	//save orbit signatures as a matrix
	ofstream fout(databaseDir + graphName + "nodeOrbitMembershipBitVector.txt");
	fout << "k = " << k_ << " numSamples = " << numSamples << endl;
	for(auto node: orbitSignature){
		for(auto id: node) fout << id;
		fout << endl;
	}
	fout.close();
}

Graphette* Database::getCanonicalGraphette(Graphette* x){
	ullint num = x->decimalNumber();
	Graphette* y = new Graphette(k_, g[num].canonicalDecimal);
	for(ullint i = 0; i < k_; i++){
		y->setLabel(g[num].canonicalPermutation[i] - '0', x->label(i));
	}
	return y;
}