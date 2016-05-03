#include "nsga-ii.h"

#include "Alignment.h"
#include "Network.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
using namespace std;

//this function assumes fitnesses have already been assigned
//todo: add check that that's the case.
vector<vector<Alignment*> > nonDominatedSort(const vector<Alignment*>& in){

	vector<vector<Alignment*> > fronts(1); //know there is at least one front.
	for(int i = 0; i < in.size(); i++){
		in[i]->numThatDominate = 0;
		in[i]->dominated.clear();
		for(int j = 0; j < in.size(); j++){
			if(i == j){
				continue;
			}
			if(dominates(in[i]->fitness,in[j]->fitness)){
				in[i]->dominated.push_back(in[j]);
			}
			else if(dominates(in[j]->fitness,in[i]->fitness)){
				in[i]->numThatDominate++;
			}
		}

		if(in[i]->numThatDominate == 0){
			in[i]->domRank = 0;
			fronts[0].push_back(in[i]);
		}
	}

	int i = 0;
	while(!(fronts.size() == i || fronts[i].empty())){
		vector<Alignment*> nextFront;

		for(int j = 0; j < fronts[i].size(); j++){
			for(auto q : fronts[i][j]->dominated){
				q->numThatDominate--;
				if(q->numThatDominate == 0){
					q->domRank = i+1;
					nextFront.push_back(q);
				}
			}
		}
		i++;
		if(!nextFront.empty()){
			fronts.push_back(nextFront);
		}
	}

	int frontsTotal = 0;
	for(int i = 0; i<fronts.size(); i++){
		frontsTotal += fronts[i].size();
	}

	return fronts;

}

void normalizeFitnesses(vector<Alignment*>& in){
	//for each objective:
	for(int i = 0; i < in[0]->fitness.size(); i++){
		//compute max of this objective
		double max = 0.0;
		for(auto p : in){
			if(p->fitness[i] > max){
				max = p->fitness[i];
			}
		}

		//set normalized fitness of all alns for this objective
		for(auto p : in){
			p->fitnessNormalized.push_back(p->fitness[i]/max);
		}
	}
}

//takes a front as input and assigns crowdDist to each element
//note: results meaningless if input is not non-dominated set
//note: normalizeFitnesses MUST be called first!
void setCrowdingDists(vector<Alignment*>& in){

	//init all to zero
	for(auto i : in){
		i->crowdDist = 0.0;
	}

	int numObjs = in[0]->fitness.size();
	int numAlns = in.size();

	//for each objective m
	for(int m = 0; m < numObjs; m++){
		//sort by objective m
		sort(in.begin(), in.end(),
			[m](const Alignment* a, const Alignment* b){
				return a->fitnessNormalized[m] < b->fitnessNormalized[m];
			});

		//set boundary points to max dist
		in[0]->crowdDist = numeric_limits<double>::max();
		in[numAlns-1]->crowdDist = numeric_limits<double>::max();

		//increment crowding dist for the current objective

		double denom = in[numAlns-1]->fitnessNormalized[m] - in[0]->fitnessNormalized[m];
		for(int i = 1; i< (numAlns-1); i++){
			double numerator = in[i+1]->fitnessNormalized[m] - in[i-1]->fitnessNormalized[m];
			in[i]-> crowdDist += (numerator/denom);
		}

	}
}

//returns true if aln1 Pareto dominates aln2
bool dominates(const vector<double>& fitness1, const vector<double>& fitness2){
	bool oneBigger = false;

	for(int i = 0; i < fitness1.size(); i++){
		if(fitness1[i] < fitness2[i]){
			return false;
		}
		if(fitness1[i] > fitness2[i]){
			oneBigger = true;
		}
	}

	return oneBigger;
}

bool crowdedComp(Alignment* aln1, Alignment* aln2){
	if(aln1->domRank == -1 || aln2->domRank == -1){
		cout<<"Danger: domRank uninitialized in crowdedComp!"<<endl;
	}
	if(aln1->crowdDist < 0.0 || aln2->crowdDist < 0.0){
		cout<<"Danger: crowdDist uninitialized in crowdedComp!"<<endl;
	}
	return (aln1->domRank < aln2->domRank)
	        || (aln1->domRank == aln2->domRank &&
	        	aln1->crowdDist > aln2->crowdDist);
}


//preconditions: tournSize smaller than in
//all in elems have crowdDist and domCount calculated
//returns two alignment pointers
vector<Alignment*> binSel(RandGenT& prng,
	                      const vector<Alignment*>& in, 
	                      unsigned int tournSize){
	vector<unsigned int> indices(in.size());
	
	for(int i = 0; i < in.size(); i++){
		indices[i] = i;
	}

	shuffle(indices.begin(),indices.end(), prng);

	//grab the best of a random subset of in
	sort(indices.begin(),indices.begin()+tournSize,
		[&in](unsigned int a, unsigned int b){
			return crowdedComp(in.at(a),in.at(b));
		});

	vector<Alignment*> toReturn;
	toReturn.push_back(in.at(indices[0]));
	toReturn.push_back(in.at(indices[1]));

	return toReturn;
}

void reportStats(const vector<Alignment*>& in, 
	             const vector<fitnessName> fitnessNames, 
	             bool verbose, bool alnDiversity){

	for(int i =0; i < in[0]->fitness.size(); i++){
		double sum = 0.0;
		double max = 0.0;
		double min = numeric_limits<double>::max();
		double mean;

		for(auto p : in){
			double temp = p->fitness[i];
			if(temp > max)
				max = temp;
			if(temp < min)
				min = temp;
			sum += p->fitness[i];
		}

		mean = sum/double(in.size());
		double std_dev = 0.0;

		for(auto p : in){
			double temp = p->fitness[i] - mean;
			std_dev += temp*temp;
		}

		std_dev /= double(in.size());
		std_dev = sqrt(std_dev);
		if(verbose){
			cout<<"Max of objective "<<fitnessNameToStr(fitnessNames[i])<<" is "<<max<<endl;
			cout<<"Min of objective "<<fitnessNameToStr(fitnessNames[i])<<" is "<<min<<endl;
			cout<<"Mean of objective "<<fitnessNameToStr(fitnessNames[i])<<" is "<<mean<<endl;
			cout<<"Std. Dev. of objective "<<fitnessNameToStr(fitnessNames[i])
			    <<" is "<<std_dev<<endl;
		}
		else{
			cout<<'\t'<<min<<'\t'<<max<<'\t'<<mean<<'\t'<<std_dev;
		}
	}

	//check on pairwise alignment similarity
	if(alnDiversity){
		double simSum = 0.0;
		double minSim = 100.0;
		double maxSim = 0.0;
		for(auto p : in){
			for(auto q : in){
				if(p == q){
					continue;
				}
				else{
					double sim = alnSimilarity(p,q);
					if(sim < minSim){
						minSim = sim;
					}
					if(sim > maxSim){
						maxSim = sim;
					}
					simSum += sim;
				}
			}
		}

		if(verbose){
			cout<<"Mean pairwise aln similarity: "<<(simSum/double(in.size()*(in.size()-1)))<<endl;
			cout<<"Max pairwise similarity: "<<maxSim<<endl;
			cout<<"Min pairwise similarity: "<<minSim<<endl;
		}
		else{
			cout<<'\t'<<(simSum/double(in.size()*(in.size()-1)));
			cout<<'\t'<<maxSim;
			cout<<'\t'<<minSim;
		}
	}


}

double alnSimilarity(const Alignment* aln1, const Alignment* aln2){
	int count = 0;
	for(int i = 0; i < aln1->actualSize; i++){
		if(aln1->alnMask[i] && aln2->alnMask[i] && aln1->aln[i] == aln2->aln[i]){
			count++;
		}
	}

	int size1 = aln1->alnSize();
	int size2 = aln2->alnSize();

	if(size1 <= size2){
		return double(count)/double(size1);
	}
	else{
		return double(count)/double(size2);
	}
}