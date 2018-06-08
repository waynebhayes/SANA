#include"convert.hpp"
#include<time.h>
#include<stdio.h>
using namespace std;


int find_type(string input)
{
	regex graphml(".*(\\.xml)");
	regex gml(".*(\\.gml)");
	regex leda(".*(\\.leda)");
	regex csv(".*(\\.csv)");
	regex lgf(".*(\\.lgf)");
	regex edgelist(".*(\\.el)");
	if(regex_match(input,lgf))
	return 6;
	if(regex_match(input,graphml))
	return 1;
	if(regex_match(input,gml))
	return 2;
	if(regex_match(input,edgelist))
	return 3;
	if(regex_match(input,csv))
	return 4;
	if(regex_match(input,leda))
	return 5;

    return 0;
}

void convert_leda( string filename){
	ifstream ifs;
	ifs.open(filename,ifstream::in);
	ofstream ofs;
	ofs.open(" tmpel");
	
	
	 string line;
	cout << "!@#!@#!@#" << endl;
	 regex pattern("([a-zA-Z\\d]*)(\\s)([a-zA-Z\\d]*)(\\s)([a-zA-Z\\d]*)(\\s)\\|\\{(.*)\\}\\|(.*)");
	while(!ifs.eof()){
		getline(ifs,line);
		if( regex_match(line,pattern))
		{
			int check = 0;
			int target_start =0;
			 string start,target;
			for(unsigned int i = 0; i < line.size(); i++){
				if(isspace(line[i]) && check == 0){
					check = 1;
					start = line.substr(0,i);
					ofs<<start<<" ";
				}
				if(!isspace(line[i]) && check == 1){
					check = 2;
					target_start = i;
				}
				if(isspace(line[i]) && check == 2){
					target = line.substr(target_start,i-target_start);
					ofs<<target<< endl;
					break;
				}
			}
		}
	}
	ifs.close();
	ofs.close();
	
}





void convert_lgf( string filename){
	ifstream ifs;
	ifs.open(filename,ifstream::in);
	ofstream ofs;
	ofs.open(" tmpel");
	
	string line;
	//cout << "works" << endl;
	regex pattern("^([\\da-zA-Z]+)(\\s+)([\\da-zA-Z]+)(\\s+)(\\d+)(\\s+)(\\d+)(\\s*)$");
	//cout << "still works" << endl;
	//nodes information

		
	while(!ifs.eof()){
		getline(ifs,line);
		if(regex_match(line,pattern)){
			int check = 0;
			int target_start =0;
			unsigned int i = 0;
			while(i < line.length())
			{
				if(isspace(line[i]) && check == 0){
					check = 1;
					string start =  line.substr(0,i);
					ofs<<start<<" ";
				}
				if(!isspace(line[i]) && check == 1){
					check = 2;
					target_start = i;
				}
				if(isspace(line[i]) && check == 2){
					check = 0;
					string target = line.substr(target_start,i-target_start);
					ofs<<target<< endl;
						break;
				}
				i++;
			}
			getline(ifs,line);
		}
	}
	
	ifs.close();
	ofs.close();
}



void convert_grapgml(string inp)
{

    regex source("(.*)(source)(.*)");
    regex target("(.*)(target)(.*)");
    ifstream inFile;
    ofstream myfile;
    inFile.open(inp);
    myfile.open (" tmpel");
    if (!inFile) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
    string line;
    string sub;
    while (inFile.good()) {

	getline (inFile,line);
	if(regex_match(line,source))
{
		size_t  s_place = line.find("source=");
		string re = line.substr(s_place);
		int n = 7;
		if(re[7] == '"')
			n++;
		string sub = re.substr(n);
		char f = ' ';
                if(n == 8)
			f = '"';
		string final_source;
		for(string::iterator it=sub.begin(); it!=sub.end(); ++it)
		{
			if(*it == f)
				break;
			final_source.push_back(*it);
			
		}
		size_t  t_place = re.find("target=");
		string t_re = re.substr(t_place);
		int x = 7;
		if(t_re[7] == '"')
			x++;
		string t_sub = t_re.substr(n);
		string final_target;
		for(string::iterator tit=t_sub.begin(); tit!=t_sub.end(); ++tit)
		{
			if(*tit == f || *tit == '>'||*tit == '/' )
				break;
			final_target.push_back(*tit);
			
		}
		myfile << final_source << "  " << final_target << "\n";
    }
}
    myfile.close();
    inFile.close();

}
void convert_gml(string inp)
{    
    clock_t t;
    t = clock();
    regex source("(.*)(source)(.*)");
    regex target("(.*)(target)(.*)");
    ifstream inFile;
    ofstream myfile;
    inFile.open(inp);
    myfile.open (" tmpel");
	t = clock() - t;
    cout << "open file needs: " <<  t << endl;
    clock_t t1;
    t1 = clock();
    if (!inFile) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }
    string line;
    string sub;
    while (inFile.good()) {

	getline (inFile,line);
	if(regex_match(line,source))
		
		myfile << line.substr(9) << ' ';
	if(regex_match(line,target))
		myfile << line.substr(9) << "\n";
continue;
	


    }
	t1 = clock() - t1;
     cout << "write file needs: " << t1 << endl;
    myfile.close();
    inFile.close();

}




void convert_csv( string filename){
	ifstream ifs;
	ifs.open(filename, ifstream::in);
	ofstream ofs;
	ofs.open(" tmpel");
	
	string line;
	regex pattern("(.*);(.*)");
	
	while(!ifs.eof()){
		getline(ifs,line);
		string start,target;
		if(regex_match(line,pattern)){
			size_t semi = line.find(";");
			target = line.substr(semi);
			start = line.substr(0,line.length()-target.length());
			target = target.substr(1,target.length()-1);
			ofs<<start<<" ";
			ofs<<target<< endl;
		}
	}
	ifs.close();
	ofs.close();
}

std::map<int,std::string> toString(std::string filename){

	clock_t t;
    t = clock();
	std::ifstream ifs;
	ifs.open(filename,std::ifstream::in);
	std::ofstream ofs;
	ofs.open("data/hashed.txt");
	
	std::vector<int> Nodes;
	std::vector<int>::iterator it;
	std::map<std::string,int> ref;
	std::map<int,std::string> result;
	int start = 0;
	
	std::string line;
	std::regex pattern(".*");
	
	while(!ifs.eof()){
		getline(ifs,line);
		if(std::regex_match(line,pattern)){
			std::size_t pos = line.find(" ");

			if(pos != std::string::npos)
			{
				//std::cout<<line.substr(0,pos);
				//std::cout<<line.substr(pos+1);
				std::string firstNode = line.substr(0,pos);
				std::string secondNode = line.substr(pos+1);
				
				if(ref.find(firstNode) == ref.end()){
					//std::cout<<"Adding FirstNode"<<std::endl;
					Nodes.push_back(start);
					ref[firstNode] = start;
					result[start] = firstNode;
					start++;
				}
				if(ref.find(secondNode) == ref.end()){
					//std::cout<<"Adding secondNode"<<std::endl;
					Nodes.push_back(start);
					ref[secondNode] = start;
					result[start] = secondNode;
					start++;
				}
				ofs<<ref[firstNode]<<" "<<ref[secondNode]<<std::endl;
			}
		}
		
	}
	ifs.close();
	ofs.close();
	t = clock() - t;
cout << "the make pairs : "<< t << endl;
	return result;
}



string  convert(string filename)
 {

	string result = " tmpel";
    switch(find_type(filename))
{
	
	case 0: 
	{cout << "Wrong" << endl;
		exit(1);}
	case 1: 
	{ 	convert_grapgml(filename);
		break;}
	case 2: 
	{	convert_gml(filename);
		break;}
	case 3:
	{
		return filename;
	}
	case 4: 
	{	convert_csv(filename);
		break;}
	case 5: 
	{	convert_leda(filename);
		break;}
	case 6: 
	{	convert_lgf(filename);
		break;}

}

    return result;
}

void delete_tmp()
{
remove("tmpel");

}
