#include "Alignment.h"
#include "blastinfo.h"
#include "Network.h"

#include <assert.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <limits>
#include <queue>
#include <utility>
using namespace std;

//todo: note these assume net2 is larger. Ensure that when loading nets.
//this constructor just creates an arbitrary non-random alignment.
//Make it a random one by calling shuf()
Alignment::Alignment(const Network* n1, const Network* n2,
	                 const BLASTDict* bit, const GOCDict* goc){
	unsigned int size = n2->nodeToNodeName.size();
	aln = vector<node>(size,-1);
	alnInv = vector<node>(size,-1);
	alnMask = vector<bool>(size,true);
	fitnessValid = false;

	for(int i = 0; i < size; i++){
		aln[i] = i;
		alnInv[i] = i;
	}

	net1 = n1;
	net2 = n2;
	domRank = -1;
	crowdDist = -1.0;
	bitscores = bit;
    gocs = goc;
	actualSize = net1->nodeToNodeName.size();
	//initialize conserved counts for fast ICS computation
	double denom = fastICSDenominator();
	currConservedCount = ics()*denom;
	currInducedCount = (int)denom;
}

Alignment::Alignment(const Network* n1, const Network* n2, 
	                 string filename,
	                 const BLASTDict* bit, const GOCDict* goc){
	int size = n2->nodeToNodeName.size();
    //todo: nodeNameToNode is sometimes a different size than 
    //nodeToNodeName and I can't figure out why.
	aln = vector<node>(size,-1);
	alnInv = vector<node>(size,-1);
	alnMask = vector<bool>(size,true);
	fitnessValid = false;
	domRank = -1;
	crowdDist = -1.0;
	bitscores = bit;
    gocs = goc;
	net1 = n1;
	net2 = n2;
	actualSize = net1->nodeToNodeName.size();
	ifstream infile(filename);
	if(!infile){
		throw LineReadException(string("Alignment file ")+filename+
			                    string(" failed to open!"));
	}
	string line;
	//keep track of which nodes in V2 aren't aligned so we can
	//add them to the permutation somewhere after loading the file.
	unordered_set<node> v1Unaligned;
	unordered_set<node> v2Unaligned;
	for(int i = 0; i < net2->nodeToNodeName.size(); i++){
		v2Unaligned.insert(i);
	}
    assert(v2Unaligned.size() == aln.size());
    
    int count = 0;
	//process each line
	while(getline(infile,line)){
		//cout<<"parsing line: "<<line<<endl;
		istringstream iss(line);
		string a, b;
		node u, v;

		

		if (!(iss >> a >> b)){
			throw LineReadException(string("Parse error in network: ") + filename +
				                   string("on line: ") + line + "\n");
		
		}
		
        if(!net1->nodeNameToNode.count(a)){
            continue;
        }
		u = net1->nodeNameToNode.at(a);
        
		if(!net2->nodeNameToNode.count(b)){
            continue;
		}
		v = net2->nodeNameToNode.at(b);
		
        count++;
		aln[u] = v;
		alnInv[v] = u;
		v2Unaligned.erase(v);
		//cout<<"node "<<a<<" (int: "<<u<<") aligned to node "
		//    <<b<<" (int: "<<v<<")"<<endl;
	
	}

	//look for unaligned nodes in V1 and align them to unaligned nodes
	//in V2. 
	for(int i = 0; i < aln.size(); i++){
		if(aln[i] == -1){
			node arbNode = *(v2Unaligned.begin());
			aln[i] = arbNode;
			alnInv[arbNode] = i;
			alnMask[i] = false;
			v1Unaligned.insert(i);
			v2Unaligned.erase(arbNode);
		}
	}

	//initialize conserved counts for fast ICS computation
	double denom = fastICSDenominator();
	currConservedCount = ics()*denom;
    currInducedCount = (int)denom;
	if(bitscores)
		currBitscore = sumBLAST();
        
    if(gocs)
        currGOC = sumGOC();
}

/*
This does UPMX crossover in a (hopefully) more efficient way. An existing
alignment becomes the child of two alignments. Standard UPMX modifies the
two existing alignments in-place. Since our algorithm is elitist, however,
we need those old alignments. Instead, the given alignment becomes what one
of those two resulting alignment would be without modifying the parents.
Which modified parent to become is chosen at random.

todo: add secondary crossover op for changing mask. Or something.
todo: this currently ignores the mask of the parents and maintains
      the mask of the current alignment.
*/
Alignment::Alignment(RandGenT& prng, float cxswappb, 
	                        const Alignment& p1,
		                    const Alignment& p2,
		                    bool total){
	const Alignment* par1;
	const Alignment* par2;
	int size = p1.aln.size();
	uniform_int_distribution<int> coinFlip(0,1);
	uniform_real_distribution<float> fltgen(0.0,1.0);
	if(coinFlip(prng)){
		par1 = &p1;
		par2 = &p2;
	}
	else{
		par1 = &p2;
		par2 = &p1;
	}

	vector<node> par1Indices(size);
	//node par1Indices[size]; //todo: benchmark this (and also test it!)
	for(int i = 0; i < size; i++){
		par1Indices[par1->aln[i]] = i;
	}

	//do standard initialization
	aln = par1->aln;
	alnInv = par1->alnInv;
	alnMask = par1->alnMask;
	fitnessValid = false;
	domRank = -1;
	numThatDominate = -1;
	crowdDist = -1.0;
	bitscores = par1->bitscores;
    gocs = par1->gocs;
	actualSize = par1->actualSize;
	currBitscore = par1->currBitscore;
    currGOC = par1->currGOC;
	currConservedCount = par1->currConservedCount;
	currInducedCount = par1->currInducedCount;
	net1 = par1->net1;
	net2 = par1->net2;



	//do crossover
	for(int i = 0; i<size; i++){
		if(fltgen(prng) < cxswappb){
			//swap nodes
			int temp1 = aln[i];
			int temp2 = par2->aln[i];
			int oldConservedI = conservedCount(i,aln[i],alnMask[i],-1);
			int oldConservedJ = conservedCount(par1Indices[temp2],
				                               aln[par1Indices[temp2]],
				                               alnMask[par1Indices[temp2]],
				                               i);
			int oldInducedI = inducedCount(aln[i],-1);
			int oldInducedJ = inducedCount(aln[par1Indices[temp2]],aln[i]);
			aln[i] = temp2;
			alnInv[temp2] = i;
			aln[par1Indices[temp2]] = temp1;
			alnInv[temp1] = par1Indices[temp2];
			//cout<<"selected to swap:"<<net1->nodeToNodeName.at(i)<<" "
				//<<net1->nodeToNodeName.at(par1Indices[temp2])<<endl;
			bool temp1Mask = alnMask[i];
			bool par1bool = alnMask[par1Indices[temp2]];
			bool par2bool = par2->alnMask[i];
			//swap mask bools
			if(!total){ //if partial alns allowed, either | or & the bit here.
				alnMask[par1Indices[temp2]] = temp1Mask;
				if(coinFlip(prng)){
					alnMask[i] = par1bool && par2bool;
				}
				else{
					alnMask[i] = par1bool || par2bool;
				}
			}

			//compute new conservedCounts
			int newConservedI = conservedCount(i, aln[i], alnMask[i],-1);
			int newConservedJ = conservedCount(par1Indices[temp2],
										   aln[par1Indices[temp2]],
										   alnMask[par1Indices[temp2]],i);
			int newInducedI = inducedCount(aln[par1Indices[temp2]],-1);
			int newInducedJ = inducedCount(aln[i],aln[par1Indices[temp2]]);

			//update currBitscore and conservedCounts
			updateBitscore(i,temp1,temp2,temp1Mask,alnMask[i]);
			updateGOC(i,temp1,temp2,temp1Mask,alnMask[i]);
			updateBitscore(par1Indices[temp2],temp2,temp1,
				           par1bool, alnMask[par1Indices[temp2]]);
			updateGOC(par1Indices[temp2],temp2,temp1,par1bool,
				      alnMask[par1Indices[temp2]]);
			currConservedCount += ((newConservedI - oldConservedI) + 
				                   (newConservedJ - oldConservedJ));
			currInducedCount += ((newInducedI - oldInducedI) + 
				                 (newInducedJ - oldInducedJ));
			//swap index records 
			int itemp = par1Indices[temp1];
			par1Indices[temp1] = par1Indices[temp2];
			par1Indices[temp2] = itemp; 
		}

	}
	fitnessValid = false;
}

//performs a fast, greedy matching based on bitscore or GOC
//does bitscores when arg is true, and GOC otherwise.
void Alignment::greedyMatch(bool bit){
	if(bit && bitscores == nullptr){
		assert(1==2);
	}

	if(!bit && gocs == nullptr){
		assert(1==2);
	}

	auto dict = bit ? bitscores : gocs;

	aln = vector<node>(net2->nodeToNodeName.size(), -1);
	alnInv = aln;
	unordered_set<node> v1Unaligned;

	for(int i = 0; i < net1->nodeToNodeName.size(); i++){
		v1Unaligned.insert(i);
	}

	unordered_set<node> v2Unaligned;
	for(int i = 0; i < net2->nodeToNodeName.size(); i++){
		v2Unaligned.insert(i);
	}

	unordered_set<node> v1Aligned, v2Aligned;

	for(auto n1 : v1Unaligned){
		node bestN2 = -1;
		double bestScore = 0.0;
		for(auto n2 : v2Unaligned){
			if((*dict)[n1][n2] > bestScore ){
				bestN2 = n2;
				bestScore = (*dict)[n1][n2];
			}
			else if(bestN2 == -1){
				bestN2 = n2;
			}
		}

		aln[n1] = bestN2;
		alnInv[bestN2] = n1;
		v1Aligned.insert(n1);
		v2Aligned.insert(bestN2);
		v2Unaligned.erase(bestN2);
		//NOTE: can't change v1Unaligned here because we're iterating over it.
		//instead changed in the for loop updating the mask below.
	}

	//update mask
	for(int i = 0; i < alnMask.size(); i++){
		if(v1Aligned.count(i)){
			alnMask[i] = true;
			v1Unaligned.erase(i);
		}
		else{
			alnMask[i] = false;
		}
	}

	//look for unaligned nodes in V1 and align them to unaligned nodes
	//in V2. 
	for(int i = 0; i < aln.size(); i++){
		if(aln[i] == -1){
			node arbNode = *(v2Unaligned.begin());
			aln[i] = arbNode;
			alnInv[arbNode] = i;
			alnMask[i] = false;
			v1Unaligned.insert(i);
			v2Unaligned.erase(arbNode);
		}
	}

	//initialize conserved counts for fast ICS computation
	double denom = fastICSDenominator();
	currConservedCount = ics()*denom;
	currInducedCount = (int)denom;

	if(bitscores)
		currBitscore = sumBLAST();

	if(gocs)
		currGOC = sumGOC();
}

void Alignment::seedExtend(bool bit, bool degdiff){
	//crash if called without data available
	if(bit && bitscores == nullptr){
		assert(1==2);
	}

	if(!bit && gocs == nullptr){
		assert(1==2);
	}

	auto dict = bit ? bitscores : gocs;

	//for priority queue, want to compare degree difference and
	//bit/goc. do degree diff as small degree/large degree so that
	//the objective needs to be maximized.

	//maybe make degree difference objective optional for ease of
	//comparison to previous seed-extend methods

	auto comp = [&](pair<node,node> x, pair<node,node> y){
		double sim1 = (*dict)[x.first][x.second];
		double sim2 = (*dict)[y.first][y.second];
		//todo: add some sort of optional degree difference comparison
		return sim1 > sim2;
	};

	priority_queue<pair<node,node>, vector<pair<node,node>>, decltype(comp) > q(comp);

	//throw everything into the queue
	for(int i = 0; i < net1->nodeToNodeName.size(); i++){
		for(int j = 0; j < net2->nodeToNodeName.size(); j++){
			q.push(pair<node,node>(i,j));
		}
	}
}

void Alignment::shuf(RandGenT& prng, bool uniformsize, 
	                 bool smallstart, bool total){
	shuffle(aln.begin(),aln.end(), prng);
	for(int i = 0; i < aln.size(); i++){
		alnInv[aln[i]] = i;
	}

	if(!total){
		if(!uniformsize && !smallstart){
			uniform_int_distribution<int> coinFlip(0,1);
			for(int i =0; i < alnMask.size(); i++){
				if(coinFlip(prng)){
					alnMask[i] = false;
				}
			}
		}
		else if(uniformsize){
			//actually randomly generating number of mask cells to
			//turn OFF because they are already all on.
			uniform_int_distribution<int> sizeDist(1,actualSize-1);
			int numToDeactivate = sizeDist(prng);
			//generate indices to flip on
			uniform_int_distribution<int> indexDist(0,actualSize-1);
			for(int i = 0; i < numToDeactivate; i++){
				int index = indexDist(prng);
				//if index already deactivated, find another
				while(!alnMask[index]){
					index = indexDist(prng);
				}
				alnMask[index] = false;
			}
			//deactivate all unaligned nodes aligned to dummy nodes
			for(int i = actualSize; i < aln.size(); i++){
				alnMask[i] = false;
			}
		}
		else{ //smallsize
			int maxsize = net1->nodeToNodeName.size() / 100;
			if(maxsize == 0){
				maxsize = 1;
			}
			uniform_int_distribution<int> sizeDist(1,maxsize);
			int numToDeactivate = actualSize - sizeDist(prng);
			uniform_int_distribution<int> indexDist(0,actualSize-1);
			for(int i = 0; i < numToDeactivate; i++){
				int index = indexDist(prng);
				while(!alnMask[index]){
					index = indexDist(prng);
				}
				alnMask[index] = false;
			}
			//deactivate all unaligned nodes aligned to dummy nodes
			for(int i = actualSize; i < aln.size(); i++){
				alnMask[i] = false;
			}
		}
	}
	if(bitscores)
		currBitscore = sumBLAST();
        
    if(gocs)
        currGOC = sumGOC();

	//initialize currConservedCount for fast ICS computation
	double denom = fastICSDenominator();
	currConservedCount = ics()*denom;
	currInducedCount = (int)denom;
}

//todo: add secondary mutate op for changing mask

void Alignment::mutate(RandGenT& prng, float mutswappb, bool total){
	int size = aln.size();
	uniform_real_distribution<float> fltgen(0.0,1.0);
	//size-2 because of increment below
	uniform_int_distribution<int> intgen(0,size-2); 
	for(int i = 0; i < aln.size(); i++){
		//swap probabilistically
		
		if(fltgen(prng) < mutswappb){
			int swapIndex = intgen(prng);
			if(swapIndex >= i){
				swapIndex++;
			}
			doSwap(i,swapIndex);
		}
		
		//switch mask bit on/off probabilistically
		
		if(!total && fltgen(prng) < mutswappb){
			int oldConserved = conservedCount(i, aln[i], alnMask[i],-1);
			int oldInduced = inducedCount(aln[i],-1);
			alnMask[i] = !alnMask[i];

			int newConserved = conservedCount(i, aln[i], alnMask[i],-1);
			int newInduced = inducedCount(aln[i],-1);
			updateBitscore(i, aln[i], aln[i], !alnMask[i],
		                   alnMask[i]);
            updateGOC(i,aln[i],aln[i],!alnMask[i],alnMask[i]);
            currConservedCount += (newConserved - oldConserved);
            currInducedCount += (newInduced - oldInduced);
		}
		
		
	}
	fitnessValid = false;
}

//takes 2 nodes from V1 and swaps the nodes they are aligned to in V2.
//updates currBitscore accordingly.
//updates conservedCounts accordingly as well.
void Alignment::doSwap(node x, node y){
	int oldConservedX = conservedCount(x, aln[x], alnMask[x],-1);
	int oldConservedY = conservedCount(y, aln[y], alnMask[y],x);
	int oldInducedX = inducedCount(aln[x],-1);
	int oldInducedY = inducedCount(aln[y],aln[x]);
	node temp = aln[x];
	aln[x] = aln[y];
	alnInv[aln[x]] = x;
	aln[y] = temp;
	alnInv[aln[y]] = y;

	bool tempb = alnMask[x];
	alnMask[x] = alnMask[y];
	alnMask[y] = tempb;

	int newConservedX = conservedCount(x,aln[x],alnMask[x],-1);
	int newConservedY = conservedCount(y,aln[y],alnMask[y],x);

	int newInducedX = inducedCount(aln[y],-1);
	int newInducedY = inducedCount(aln[x],aln[y]);

	updateBitscore(x, aln[y], aln[x], alnMask[y], alnMask[x]);
    updateGOC(x, aln[y], aln[x], alnMask[y], alnMask[x]);
	updateBitscore(y, aln[x], aln[y], alnMask[x], alnMask[y]);
    updateGOC(y, aln[x], aln[y], alnMask[x], alnMask[y]);
    currConservedCount += ((newConservedX - oldConservedX) +
    	                   (newConservedY - oldConservedY));
    currInducedCount += ((newInducedX - oldInducedX) +
    	                 (newInducedY - oldInducedY));
}

void Alignment::onBit(node x){
	int oldConserved = conservedCount(x, aln[x], alnMask[x],-1);
	int oldInduced = inducedCount(aln[x],-1);
	bool old = alnMask[x];
	alnMask[x] = true;
	updateBitscore(x,aln[x],aln[x],old, alnMask[x]);
    updateGOC(x, aln[x], aln[x], old, alnMask[x]);
    int newConserved = conservedCount(x, aln[x], alnMask[x],-1);
    int newInduced = inducedCount(aln[x],-1);
	currConservedCount += (newConserved - oldConserved);
	currInducedCount += (newInduced - oldInduced);
}

double Alignment::fastEC() const{
	return double(currConservedCount)/double(net1->edges.size());
}

double Alignment::fastS3() const{
	double num = (double)currConservedCount;
    double denom = double(net1->edges.size() 
                          + currInducedCount) - num;
    return num/denom;
}

double Alignment::icsTimesEC() const{
	return fastICS()*fastEC();
}

double Alignment::s3Variant() const{
	double a = (double)currConservedCount;
	double b = (double)(net1->edges.size());
	double c = (double)currInducedCount;
	double denom = max(b,c) -a;
	return a/denom;
}

//todo: maybe something more principled than fitnessNames (so ad hoc!)
void Alignment::computeFitness(const vector<fitnessName>& fitnessNames){

	if(fitness.size() == 0){
		fitness = vector<double>(fitnessNames.size(),0.0);
	}

	for(int i = 0; i < fitnessNames.size(); i++){

		switch(fitnessNames[i]){
			case ICSFit:
				fitness[i] = fastICS();
				break;
			case ECFit:
				fitness[i] =fastEC();
				break;
			case BitscoreSumFit:
				fitness[i] = currBitscore;
				break;
			case EvalsSumFit:
				fitness[i] = -1.0*currBitscore;
				break;
			case SizeFit:
				fitness[i] = alnSize();
				break;
			case GOCFit:
				fitness[i] = currGOC;
				break;
			case S3Fit:
				fitness[i] = fastS3();
				break;
			case S3DenomFit:
				fitness[i] = -(double(net1->edges.size() + currInducedCount)
        		              -currConservedCount);
				break;
			case ICSTimesEC:
				fitness[i] = icsTimesEC();
				break;
			case S3Variant:
				fitness[i] = s3Variant();
				break;
		}
	}
	fitnessValid = true;
}

double Alignment::ics() const{


	unordered_set<node> v1Unaligned;
	unordered_set<node> v2Unaligned; //set of unaligned nodes in V2
	for(int i = 0; i < aln.size(); i++){
		if(!alnMask[i]){
			v1Unaligned.insert(i);
			v2Unaligned.insert(aln[i]);
		}
		//second way for a node in V2 to be unaligned: by
		//being aligned to a dummy node.
		if(i >= net1->nodeToNodeName.size()){
			v2Unaligned.insert(aln[i]);
		}
	}

	//induced subgraph of net2 that has been mapped to:
	unordered_set<Edge, EdgeHash> inducedES;
	for(Edge e : net2->edges){
		if(!(v2Unaligned.count(e.u()) || v2Unaligned.count(e.v()))){
			inducedES.insert(e);
		}
	}

	unordered_set<Edge, EdgeHash> mappedNet1ES;
	for(Edge e : net1->edges){
		if(!(v1Unaligned.count(e.u()) || v1Unaligned.count(e.v()))){
			Edge mapped(aln[e.u()],aln[e.v()]);
			mappedNet1ES.insert(mapped);
		}
	}

	double denominator = double(inducedES.size());
	
	if(denominator == 0.0){
		return 0.0;
	}
	else{
		unordered_set<Edge,EdgeHash> intersect;
		if(mappedNet1ES.size() < inducedES.size()){
			for(auto it = mappedNet1ES.begin(); it != mappedNet1ES.end(); it++){
				if(inducedES.count(*it)){
					intersect.insert(*it);
				}
			}
		}
		else{
			for(auto it = inducedES.begin(); it != inducedES.end(); it++){
				if(mappedNet1ES.count(*it)){
					intersect.insert(*it);
				}
			}
		}
		double numerator = double(intersect.size());
		return numerator/denominator;
	}
}

//note: this function is probably as fast as it is going to get. 
//my attempts at optimizing it further have only slowed it down.
//todo: if the speed of this function is still unbearable,
//consider tracking it as we go like numerator.
double Alignment::fastICSDenominator() const{
	//construct set of nodes actually mapped to

	vector<bool> mapped(aln.size(),false);
	for(int i = 0; i < actualSize; i++){
		mapped[aln[i]] = alnMask[i]; 
	}

	int count = 0;
	for(int i = 0; i < mapped.size(); i++){
		if(mapped[i]){
			for(auto y : net2->adjList.at(i)){
				if(mapped[y]){
					if(i==y)
						count += 2; //self loops must count twice like others 
					else
						count++;
				}
			}
		}
	}

	return double(count / 2);
}


double Alignment::fastICS() const{
	double denom = (double)currInducedCount;
	if(denom == 0.0){
		return 0.0;
	}
	return (double(currConservedCount)) / denom;
}

double Alignment::sumBLAST() const{
	double toReturn = 0.0;

	for(node i = 0; i < actualSize; i++){
		if(alnMask[i]){
			toReturn += (*bitscores)[i][aln[i]];
		}
	}
	return toReturn;
}

double Alignment::sumGOC() const{
    double toReturn = 0.0;
    
    for(node i = 0; i < actualSize; i++){
        if(alnMask[i]){
            toReturn += (*gocs)[i][aln.at(i)];
        }
    }
    
    return toReturn;
}

double Alignment::alnSize() const{
	double toReturn = 0.0;
	for(int i = 0; i < actualSize; i++){
		if(alnMask[i]){
			toReturn += 1.0;
		}
	}
	return toReturn;
}

void Alignment::save(string filename) const{
	ofstream ofile(filename);
	for(int i = 0; i < net1->nodeToNodeName.size(); i++){
		if(alnMask[i]){
			string u = net1->nodeToNodeName.at(i);
			string v = net2->nodeToNodeName.at(aln[i]);
			ofile<<u<<' '<<v<<endl;
		}
	}
}

inline void Alignment::updateBitscore(node n1, node n2old, node n2new, 
									  bool oldMask, bool newMask){
	if(!bitscores)
		return;
	if(n1 >= actualSize)
		return;

	double oldScore = 0.0;

	//set oldScore
	if(oldMask)
		oldScore = (*bitscores)[n1][n2old];

	double newScore = 0.0;

	//set newScore
	if(newMask)
		newScore = (*bitscores)[n1][n2new];

	double delta = newScore - oldScore;
	currBitscore += delta;
}

inline void Alignment::updateGOC(node n1, node n2old, node n2new,
                                 bool oldMask, bool newMask){
    
    if(!gocs || n1 >= actualSize)
        return;
        
    double oldScore = 0.0;
    
    if(oldMask)
        oldScore = (*gocs)[n1][n2old];
        
    double newScore = 0.0;
    
    if(newMask)
        newScore = (*gocs)[n1][n2new];
        
    currGOC += newScore - oldScore;
}

//takes: n1 in V1, n2 in V2 that n1 is aligned to, mask that indicates
//whether they are actually aligned or not, and a node in V1 to ignore,
//in case conservedCount has been previously called where n1 == ignore
//so we want to avoid double-counting.
int Alignment::conservedCount(node n1, node n2, bool mask, node ignore) const{

	int toReturn = 0;

	if(!mask || n1 >= actualSize || n1 == ignore){
		return 0;
	}
	else{
		for(int ix = 0; ix < net1->adjList[n1].size(); ix++){
			int i = net1->adjList[n1][ix];
			if(i != ignore && alnMask[i] && net2->adjMatrix[n2][aln[i]]){
				toReturn++;
			}
		}
	}
		
	return toReturn;
}

//returns the number of edges in the subgraph induced by our alignment that
//are incident to n2 but not incident to ignore.
int Alignment::inducedCount(node n2, node ignore) const{
	if(alnInv[n2] >= actualSize || alnMask[alnInv[n2]] == false){
		return 0;
	}
	else{
		int toReturn = 0;
		for(int ix = 0; ix < net2->adjList[n2].size(); ix++){
			node x = net2->adjList[n2][ix];
			if(alnInv[x] < actualSize && alnMask[alnInv[x]] && x != ignore){
				toReturn++;
			}
		}
		return toReturn;
	}
}
