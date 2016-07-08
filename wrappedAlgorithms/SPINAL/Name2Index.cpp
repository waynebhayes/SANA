#include <fstream>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

int search(vector<string> input, string str){
    for(uint i = 0; i < input.size(); i++){
        if(input.at(i) == str){
            return i;
        }
    }
    return -1;
}

int main(int argc, const char* argv[]){ //correct order: x.gw y.gw three_column_sequence_file_space_separated
    if(argc != 4 || argv[1] == "--help"){cout << "Wrong style of input. Correct format is \"./Name2Index <.gw file associated with the first column of the sequence file> <.gw file associated with the second column> <sequence file>\"" << endl; return -1;}
    ifstream x;
    ifstream y;
    ifstream s;

    int xsize;
    int ysize;
    
    string xline;
    string yline;
    string sline;

    x.open(argv[1]);
    y.open(argv[2]);
    s.open(argv[3]);

    for(uint i = 0; i < 5; i++){getline(x, xline); getline(y, yline);}
    xsize = stoi(xline);
    ysize = stoi(yline);
    
    vector<string> xnames(0);
    vector<string> ynames(0);
    
    
    for(uint i = 0; i < xsize; i++){
        getline(x, xline);
        string xname = xline.substr(2,xline.find("}")-2);
        xnames.push_back(xname);
    }
    
    for(uint i = 0; i < ysize; i++){
        getline(y, yline);
        string yname = yline.substr(2,yline.find("}")-2);
        ynames.push_back(yname);
    }

    int iteration = 0;
    while(getline(s, sline)){
        string xpartstr;
        string ypartstr;
        string score;
        int tabpos1 = sline.find("\t");
        int tabpos2 = sline.find("\t", tabpos1+1);
        int tabpos3 = sline.find("\t", tabpos2+1);
        xpartstr = sline.substr(0, tabpos1);
        ypartstr = sline.substr(tabpos1+1, tabpos2-tabpos1-1);
        score = sline.substr(tabpos2);
        int xpart = search(xnames, xpartstr);
        int ypart = search(ynames, ypartstr);
        if(xpart == -1 || ypart == -1){cerr << "Naming mismatch, see line " << iteration << endl; return -1;}
        cout << xpart << " " << ypart << " " << score << endl;
        iteration++;
    }
    return 1;
}
