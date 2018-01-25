#include "SANA.hpp"
#include "SANAResult.hpp"
#include "SANAConfiguration.hpp"
#include "Graph.hpp"
#include "Utility.hpp"
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Input must include two or more LEDA Graph file\n";
        return 1;
    }
    vector <Graph> networkList;
    for (int i = 1; i < argc; i++) {
        networkList.push_back(Utility::LoadGraphFromLEDAFile(argv[i]));
        //networkList.push_back(Utility::LoadGraphFromEdgeList(argv[i]));
    }

    SANAConfiguration sanaConfig;
    SANA sana(sanaConfig);
    SANAResult results = sana.StartAligner(networkList);
}
