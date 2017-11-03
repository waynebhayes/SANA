#include <fstream>
#include <chrono>
#include "graphette.hpp"
#include <string>

using namespace std;

int main(int arg, char* argv[])
{
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    
    int k = atoi(argv[1]);
    if(k>=8) {
        cerr << "you choose k >= 8; this will require at least 600GB of RAM. Aborting.\n";
        exit(1);
    }

    ofstream fcanon("list"+to_string(k)+".txt"), fmap("canon_map"+to_string(k)+".txt");

    vector<Graph*> graph_vectors = generate_all_graphs(k);
    vector<Graph*> graph_canonical = generate_canonical(graph_vectors);
    fcanon << graph_canonical.size() << "\n";

    // This will show the decimal representations of adjMatrix of each Graph
    for (Graph* g : graph_canonical)
        fcanon << g->get_decimal_representation() << " ";
    fcanon << "\n";

    cout << "Writing map from all -> canonicals... (this could take awhile)";cout.flush();
    for(auto g: graph_vectors){
        Graph* cg = get_canonical(g, graph_canonical);
        fmap << cg->get_decimal_representation() << " ";
    }
    cout << "done!" << endl;
    fmap << "\n";
    // Deallocate all Graphs
    for (unsigned int i = 0; i < graph_vectors.size(); i++)
    {
        delete graph_vectors[i];
    }

    // Output the time the program takes to run in seconds
    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(t2 - t1).count();                
    cout << "Time: " << duration << " seconds" << endl;                                 
    
    return 0;
}
