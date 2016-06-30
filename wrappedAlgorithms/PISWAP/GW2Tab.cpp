#include <vector>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]){
	ifstream x;
	ifstream y;
	ofstream x1;
	ofstream y1;
	int xsize, ysize;
	x.open(argv[1]);
	y.open(argv[2]);
	x1.open(argv[3]);
	y1.open(argv[4]);
	string xline, yline;
	vector<string> xnames(0); 
	vector<string> ynames(0);

	for(uint i = 0; i < 5; i++){getline(x, xline); getline(y, yline);}
	xsize = stoi(xline);
	ysize = stoi(yline);

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

	getline(x,xline);
	getline(y,yline);
	x1 << "INTERACTOR_A\tINTERACTOR_B" << endl;
	y1 << "INTERACTOR_A\tINTERACTOR_B" << endl;
	while(getline(x, xline)){
		string firstpartstr = xline.substr(0, xline.find(" "));
		string secondpartstr = xline.substr(xline.find(" ")+1, xline.find(" ",xline.find(" ")+1));
		int firstnum = stoi(firstpartstr)-1;
		int secondnum = stoi(secondpartstr)-1;
		x1 << xnames.at(firstnum) << "\t" << xnames.at(secondnum) << endl;
	}

	while(getline(y, yline)){
       		string firstpartstr = yline.substr(0, yline.find(" "));
                string secondpartstr = yline.substr(yline.find(" ")+1, yline.find(" ",yline.find(" ")+1));
                int firstnum = stoi(firstpartstr)-1;
                int secondnum = stoi(secondpartstr)-1;
                y1 << ynames.at(firstnum) << "\t" << ynames.at(secondnum) << endl;
        }	
}
