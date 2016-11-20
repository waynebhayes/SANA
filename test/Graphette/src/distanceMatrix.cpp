#include "distanceMatrix.hpp"
using namespace std;

void distanceMatrix(uint N, uint K)
{
    uint input, canon_size, numEdges = (N*N-N)/2;
    if(K > numEdges) 
        throw domain_error("Hamming distance must be <= Number of possible edges");
    else if (K == 0)
        throw domain_error("Hamming distance cannot be 0");

    string canon_filepath("data/list"+to_string(N)+".txt");
    string map_filepath("data/canon_map"+to_string(N)+".txt");

    vector<uint> canons, canon_map;

    //Getting the inputs.
    ifstream fin;
    fin.open(canon_filepath);
    fin >> canon_size;
    while(fin >> input){
        canons.push_back(input);
    }
    fin.close();
    
    fin.open(map_filepath);
    while(fin >> input){
        canon_map.push_back(input);
    }
    fin.close();

    string output = "data/HammingDistance"+to_string(N)+".txt";
    ofstream fout(output);

    //For each of the canonical graphs, we shall determine all the cannonical graphettes 
    //that are K-hamming distance away from it
    for(uint k = 1; k <= K; k++){
        vector<vector<bool>> dmatrix(canon_size, vector<bool> (canon_size, 0));
        for(uint i = 0; i < canons.size(); i++){
            Graphette g(N, canons[i]);
            vector<bool> bits = g.bitVector();        
            
            //this string is used to determine all numEdges chooose K combinations. 
            string bitmask(k, 1); // K leading 1's
            bitmask.resize(numEdges, 0); // N*N-N-K trailing 0's
            do {
                vector<bool> copy = bits;
                for (uint i = 0; i < bitmask.size(); i++){
                    if(bitmask[i]){
                        copy[i] = !copy[i]; //changing bits
                    }
                }
                Graphette h(N, copy);
                uint newNum = h.decimalNumber();
                uint c = canon_map[newNum];
                auto temp = find(canons.begin(), canons.end(), c);
                uint j = temp-canons.begin(); //j is the index of c in the list of canonical graphs
                dmatrix[i][j] = true;
                dmatrix[j][i] = true;
                
            } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
        }
        fout << k << endl;
        for(auto row: dmatrix){
            for(auto entry: row)
                fout << entry << " ";
            fout << "\n";
        }
    }
    fout.close();
}