#include "localSearch.h"

#include "Alignment.h"
#include "Network.h"
#include "nsga-ii.h"
#include <random>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <algorithm>
using namespace std;

//instead of using the buggy hypotheticals, do an actual swap and
//check for improvement, undoing if worse.
//when obj is specified, only checks for improvement in objective obj
void correctHillClimb(RandGenT& prng, Alignment* aln, bool total,
					  int maxIters, const vector<fitnessName>& fitnessNames,
					  int obj){
	auto randNonDummyIndex = uniform_int_distribution<int>(0, aln->actualSize-1);
	auto randIndex = uniform_int_distribution<int>(0,aln->aln.size()-2);
	for(int i = 0; i < maxIters; i++){
		node x = randNonDummyIndex(prng);
		node y = randIndex(prng);
		if(y >= x){
			y++;
		}
		double currFit[aln->fitness.size()];
		copy(aln->fitness.begin(), aln->fitness.end(), currFit);

		aln->doSwap(x,y);
		aln->computeFitness(fitnessNames);

		bool improved = true;
		if(obj == -1){
			for(int j = 0; j < aln->fitness.size(); j++){
				if(aln->fitness[j] < currFit[j]){
					improved = false;
				}
			}
		}
		else{
			if(aln->fitness[obj] <= currFit[obj]){
				improved = false;
			}
		}

		//if not an improvement, undo the swap
		if(!improved){
			aln->doSwap(x,y);
			aln->computeFitness(fitnessNames);
		} 
	}
}

//optimizes given objective with time proportional to given proportion,
//and evenly distributes the rest of hillclimbing time to the other
//objectives.
void proportionalSearch(RandGenT& prng, Alignment* aln, bool total,
	                    int iters, const vector<fitnessName>& fitnessNames,
	                    int obj, double proportion){

	auto prob = uniform_real_distribution<double>(0,1);

	auto randObj = uniform_int_distribution<int>(0,fitnessNames.size()-1);

	for(int i = 0; i < iters; i++){
		double res = prob(prng);
		if(res < proportion){
			correctHillClimb(prng, aln, total,
	               500, fitnessNames,obj);
		}
		else{
			int robj = obj;
			while(robj == obj){
				robj = randObj(prng);
			}
			correctHillClimb(prng, aln, total,
	               500, fitnessNames,robj);	
		}
	}

}

//returns -1.0 if any objectives are worsened. 
//Otherwise, returns magnitude of pct improvement vector
double swapNormalizedDelta(Alignment& aln, const vector<fitnessName>& 
						   fitnessNames, node x, node y){
	vector<double> currFit = aln.fitness;

	aln.doSwap(x,y);
	aln.computeFitness(fitnessNames);
	vector<double> newFit = aln.fitness;
	aln.doSwap(x,y);
	aln.computeFitness(fitnessNames);

	vector<double> pctDeltas;

	bool oneNeg = false;
	for(int i = 0; i < newFit.size(); i++){
		double pctDelt = (newFit[i] - currFit[i])/currFit[i];
		pctDeltas.push_back(pctDelt);
		if(pctDelt < 0){
			oneNeg = true;
		}
	}

	if(oneNeg){
		return -1.0;
	}
	else{
		double sumSq = 0.0;
		for(int i = 0; i < pctDeltas.size(); i++){
			sumSq += pctDeltas[i]*pctDeltas[i];
		}

		return sqrt(sumSq);
	}
}

void steepestAscentHillClimb(Alignment* aln, 
							 vector<fitnessName>& fitnessNames,
							 int nthreads, bool verbose){

	vector<node> bestXs(nthreads,-1);
	vector<node> bestYs(nthreads,-1);
	vector<double> bestDeltas(nthreads,-1.0);

	auto worker = [&](int tid, int xmin, int xmax, int ymin, int ymax){
		Alignment localAln(*aln);
		node bestX = -1;
		node bestY = -1;
		double bestDelta = -1.0;

		for(node x = xmin; x < xmax; x++){
			for(node y = ymin; y < ymax; y++){
				double newDelta = 
					swapNormalizedDelta(localAln, fitnessNames,
										x,y);
				if(newDelta > bestDelta){
					bestDelta = newDelta;
					bestX = x;
					bestY = y;
				}
			}
		}

		bestXs[tid] = bestX;
		bestYs[tid] = bestY;
		bestDeltas[tid] = bestDelta;
	};

	bool done = false;
	int numiters = 0;
	while(!done){
		//launch worker threads
		int grainsize = aln->actualSize / nthreads;
		vector<thread> ts;
		for(int tid = 0; tid < nthreads; tid++){
			int tminx = tid*grainsize;
			int tmaxx;
			if(tid == nthreads-1){
				tmaxx = aln->actualSize;
			}
			else{
				tmaxx = tminx + grainsize;
			}
			ts.push_back(thread(worker,tid,tminx,tmaxx,0,aln->aln.size()));
		}

		//join threads
		for(int i = 0; i < nthreads; i++){
			ts.at(i).join();
		}

		//find absolute best swap and commit to it
		node bestX = -1;
		node bestY = -1;
		double bestDelt = -1.0;

		for(int i = 0; i < bestDeltas.size(); i++){
			if(bestDeltas.at(i) > bestDelt){
				bestDelt = bestDeltas.at(i);
				bestX = bestXs.at(i);
				bestY = bestYs.at(i);
			}
		}

		if(bestDelt <= 0.0){
			done = true;
		}
		else{
			aln->doSwap(bestX, bestY);
			aln->computeFitness(fitnessNames);
			if(verbose){
				reportStats({aln}, fitnessNames, true, false);
				cout<<(++numiters)<<" swaps performed."<<endl;
			}
		}

	}
}
/*
void potentialBasedSearch(RandGenT& prng, Alignment* aln, bool total,
						  int iters, const vector<string>& fitnessNames,
						  bool bit){
	//set up whether to use goc or bit
	if(!aln->bitscores && !aln->gocs){
		cout<<"Error: potentialBasedSearch requires bitscores or GOC"<<endl;
	}

	auto dict = bit ? aln->bitscores : aln->gocs;

	//memoize the max bitscore/goc each node could have
	vector<double> maxBit(aln->actualSize,0.0);

	for(int i = 0; i < maxBit.size(); i++){
		double best = 0.0;
		for(auto pair : dict->at(i)){
			if(pair.second > best){
				best = pair.second;
			}
		}
		maxBit.at(i) = best;
	}

	for(int i = 0; i < iters; i++){

		//get current conserved count and bitscore
		//for each aligned pair
		//todo: lots of wasted work here between iters
		vector<double> currConservedCounts(aln->actualSize);
		vector<double> currBits(aln->actualSize,0.0);

		for(int x = 0; x < aln->actualSize; x++){
			double currConserved 
				= aln->conservedCount(x,aln->aln[x],aln->alnMask[x],-1);
			currConservedCounts.at(x) = currConserved;
			if(dict->count(x) && dict->at(x).count(aln->aln[x])){
				currBits.at(x) = dict->at(x).at(aln->aln[x]);
			}
		}

		vector<node> nodes(aln->actualSize);
		for(int j = 0; j < aln->actualSize; j++){
			nodes.at(j) = j;
		}

		vector<double> potentials(aln->actualSize);

		for(int j = 0; j < potentials.size(); j++){
			double ecPotent = 1.0 - (currConservedCounts.at(j)/aln->net1->degree(j));
			double bitPotent = 1.0 - (currBits.at(j)/maxBit.at(j));
			potentials.at(j) = sqrt(ecPotent*ecPotent + bitPotent*bitPotent);
		}

		//sort nodes by potential
		sort(nodes.begin(), nodes.end(), [&](node x, node y){
			return potentials.at(x) > potentials.at(y);
		});

		bool goodSwap = false;

		uniform_int_distribution<int> topHalf(0,potentials.size()/2 - 1);

		while(!goodSwap){
			node x = topHalf(prng);
			node y = x;
			while(y == x){
				y = topHalf(prng);
			}
			vector<double> currFit = aln->fitness;
			aln->doSwap(x,y);
			aln->computeFitness(fitnessNames);
			vector<double> newFit = aln->fitness;

			bool improved = true;
			for(int j = 0; j < newFit.size(); j++){
				if(newFit.at(j) < currFit.at(j)){
					improved = false;
				}
			}

			if(!improved){
				aln->doSwap(x,y);
				aln->computeFitness(fitnessNames);
			}
			else{
				goodSwap = true;
			}
		}

	}

}
*/
VelocityTracker::VelocityTracker(){
	nextSpot = 0;
	size = 0;

	recentDeltas = vector<vector<double> >(500);

}

void VelocityTracker::reportDelta(const vector<double>& in){
	recentDeltas[nextSpot] = vector<double>(in);
	nextSpot = (nextSpot + 1) % 500;

	if(size < 500){
		size++;
	}
}

vector<double> VelocityTracker::getRecentVel() const{
	vector<double> toReturn(recentDeltas[0].size(), 0.0);
	
	for(int i = 0; i < size; i++){
		for(int j = 0; j < recentDeltas[i].size(); j++){
			toReturn[j] += recentDeltas[i][j];
		}
	}
	
	double n = double(size);
	for(int j = 0; j < toReturn.size(); j++){
		toReturn[j] /= n;
	}

	return toReturn;
}