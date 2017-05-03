#include "reversePermutation.hpp"

using namespace std;

void extractCanoonicals(uint k){
    ifstream fin("data/poscanon_map"+to_string(k)+".txt");
    ofstream fout("data/poscanon_list"+to_string(k)+".txt");
    string perm;
    int canon, count = 0;
    int num = (pow(2, (k*k-k)/2)+0.5);
    vector<bool> canons(num,false);
    while(fin >> canon >> perm){
        canons[canon] = true;
    }
    fin.close();
    for(int i=0; i<num; i++) 
        if(canons[i]) count++;
    fout << count << endl;
    for(int i=0; i<num; i++){
        if(canons[i]) fout << i << " ";
    }
}

string reverseperm(string original){
    string output = original;
    for(uint i=0; i< original.length(); i++){
        int index = original.at(i)-'0';
        output[index] = (i+'0');
    }
    return output;
}

void convertData(uint k){
    uint g, cg;
    string perm;
    ifstream fin("data/newpermutation_map"+to_string(k)+".txt");
    ofstream fout("data/poscanon_map"+to_string(k)+".txt");
    uint num = (pow(2, (k*k-k)/2)+0.5);
    vector<uint> canon_list(num);
    vector<string> canon_perm(num);
    while(fin >> g >> cg >> perm){
        canon_list[g] = cg;
        canon_perm[g] = reverseperm(perm);
    }
    for(uint i = 0;  i < num; i++){
        fout << canon_list[i] << " " << canon_perm[i] << endl;
    }
    fin.close();
    fout.close();
}
