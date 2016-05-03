#pragma once

#include "Alignment.h"
#include "Network.h"

#include <random>
#include <vector>

void correctHillClimb(RandGenT& prng, Alignment* aln, bool total,
					  int maxIters, const vector<fitnessName>& fitnessNames,
					  int obj = -1);

void proportionalSearch(RandGenT& prng, Alignment* aln, bool total,
	                    int iters, const vector<fitnessName>& fitnessNames,
	                    int obj, double proportion);

//this does a swap, computes the % change in each objective,
//undoes the swap, and reports the length of the % change vector
double swapNormalizedDelta(Alignment& aln, const vector<fitnessName>& 
						   fitnessNames, node x, node y);

void steepestAscentHillClimb(Alignment* aln, 
							 vector<fitnessName>& fitnessNames,
							 int nthreads, bool verbose);
/*
void potentialBasedSearch(RandGenT& prng, Alignment* aln, bool total,
						  int iters, const vector<string>& fitnessNames,
						  bool bit);
*/
class VelocityTracker{
public:
	VelocityTracker();
	void reportDelta(const vector<double>& in);
	vector<double> getRecentVel() const;

private:
	int nextSpot;
	int size;
	vector<vector<double> > recentDeltas;
};