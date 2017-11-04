#include "Database.hpp"
#include "utils/fileno.hpp"

using namespace std;

Database::Database()
    : Database(5)
{}

#define MIN(a,b) ((a)<(b)?(a):(b))

Database::Database(short k)
    : k_(k)
{
    /**Just reading the data from canon_map, canon_list, and orbit_map**/
    ullint m, decimal;
    string permutation;
    struct rlimit rlim;
    if(getrlimit(RLIMIT_NOFILE, &rlim)<0){perror("getrlimit(2)"); exit(1);}
    rlim.rlim_cur=rlim.rlim_max;
    if(setrlimit(RLIMIT_NOFILE, &rlim)<0){perror("setrlimit(2)"); exit(1);}
    if(getrlimit(RLIMIT_NOFILE, &rlim)<0){perror("getrlimit(2)"); exit(1);}
    //MAX_FD=MIN(80000,rlim.rlim_cur);
    //cerr << "MAX_FD is " <<MAX_FD <<endl;
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
        for(short j = 0; j < k; j++){
            forbit_map >> ids[j];
        }
        orbitId_.push_back(ids);
    }
    fcanon_list.close();
    fcanon_map.close();
    forbit_map.close();
}

void Database::addGraph(string filename, long long int numSamples){
    bool printOrbits = false;
    if(numSamples < 0) {
        numSamples = -numSamples;
        printOrbits=true;
    }
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
    ullint indicator = numSamples / 10;
    for(long long int i = 0; i < numSamples; i++){
        if(i % indicator == 0)
            cerr << (i / indicator) * 10 << "% complete\n";
        auto edge = xrand(0, edgelist.size());
        Graphette* x = graph.sampleGraphette(k_, edgelist[edge].first, edgelist[edge].second);
        Graphette* gCanon = this->getCanonicalGraphette(x);
        delete x;
        auto gCanonInt = gCanon->decimalNumber();
        cout << gCanonInt << '\t'; // output the integer bitmatrix of the canonical
        ullint l = lower_bound(canonicalGraphette.begin(), canonicalGraphette.end(), gCanonInt) - canonicalGraphette.begin();
        // cout << l << '\t'; // output the ordinal of the canonical
        cout << gCanon->label(0);
        for(ullint j = 1; j < k_; j++){
            cout << '\t' << gCanon->label(j);
            //orbitSignature[gCanon->label(j)][id]= true;
        }
        cout << endl;
        if(printOrbits) {
            for(ullint j = 0; j < k_; j++){
                ullint id = orbitId_[l][j];
                cout  << '\t' << id << '\t' << gCanon->label(j);
                for(auto orbit: gCanon->labels()){
                    if(orbit != gCanon->label(j))
                        cout << '\t' << orbit;
                }
                cout << endl;
            }
        }
        delete gCanon;
    }
    cerr << "100% complete\n";
}

Graphette* Database::getCanonicalGraphette(Graphette* x){
    ullint num = x->decimalNumber();
    Graphette* y = new Graphette(k_, g[num].canonicalDecimal);
    for(ullint i = 0; i < k_; i++){
        y->setLabel(g[num].canonicalPermutation[i] - '0', x->label(i));
    }
    return y;
}
