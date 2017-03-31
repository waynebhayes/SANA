#include "Network.h"

#include <unordered_set>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
using namespace std;


string fitnessNameToStr(fitnessName x){
	switch(x){
		case ICSFit:
			return "ICS";
		case ECFit:
			return "EC";
		case BitscoreSumFit:
			return "BitscoreSum";
		case EvalsSumFit:
			return "EvalsSum";
		case SizeFit:
			return "Size";
		case GOCFit:
			return "GOC";
		case S3Fit:
			return "S3";
		case S3DenomFit:
			return "S3Denom";
		case ICSTimesEC:
			return "ICS*EC";
		case S3Variant:
			return "S3Variant";
		default:
			return "invalid fitness name";
	}
}

Network::Network(string filename){
	unsigned int count = 0;
	unordered_set<string> alreadySeen;
	adjList = vector<vector<node> >();
	ifstream infile(filename);
	if(!infile){
		throw LineReadException(string("Given network file ")+filename+
			                    string(" failed to open!"));
	}
	string line;
	while(getline(infile,line)){
		istringstream iss(line);
		string a, b;
		node u, v;

		if (!(iss >> a >> b)){
			throw LineReadException(string("Parse error in network: ") + filename +
				                   string("on line: ") + line + "\n");
		}

		if(!alreadySeen.count(a)){
			alreadySeen.insert(a);
			nodeToNodeName[count] = a;
			nodeNameToNode[a] = count;
			auto toPushBack = vector<node>();
			adjList.push_back(toPushBack);
			u = count;
			count++;
		}
		else{
			u = nodeNameToNode.at(a);
		}

		if(!alreadySeen.count(b)){
			alreadySeen.insert(b);
			nodeToNodeName[count] = b;
			nodeNameToNode[b] = count;
			auto toPushBack = vector<node>();
			adjList.push_back(toPushBack);
			v = count;
			count++;
		}
		else{
			v = nodeNameToNode.at(b);
		}

		Edge e = Edge(u,v);

		edges.insert(e);

		adjList.at(u).push_back(v);
		if(u != v){
			adjList.at(v).push_back(u);
		}

	}


	//cout<<"ignored "<<ignoredSelfLoops<<" self-loops"<<endl;
	//set up adjacency matrix
	for(int i = 0; i < nodeToNodeName.size(); i++){
		adjMatrix.push_back(vector<bool>(nodeToNodeName.size(),false));
	}

	assert(nodeNameToNode.size() == adjList.size());

	for(int i = 0; i < adjList.size(); i++){
		for(int j = 0; j < adjList.at(i).size(); j++){
			adjMatrix[i][adjList.at(i).at(j)] = true;
		}
	}
    
    assert(nodeNameToNode.size() == nodeToNodeName.size());
}

int Network::degree(node x) const{
	return adjList.at(x).size();
}
