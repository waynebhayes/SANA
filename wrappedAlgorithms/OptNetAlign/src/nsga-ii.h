#pragma once

#include "Alignment.h"
#include "Network.h"
#include <vector>
#include <string>
#include <random>

/*
The functions below, taken together, can be used to implement
all of NSGA-II. This entails:
-doing a non-dominated sort of the population
-for each front, assigning crowding distances
-sorting by the crowded comparison operator
*/


vector<vector<Alignment*> > nonDominatedSort(const vector<Alignment*>& in);

//precond: all alns have fitnesses computed
//postcond: all alns have fitnessNormalizeds computed
//this should be called before setCrowdingDists
void normalizeFitnesses(vector<Alignment*>& in);

//in is assumed to be a front produced by nonDominatedSort
void setCrowdingDists(vector<Alignment*>& in);

//returns true iff aln1 dominates aln2
bool dominates(const vector<double>& fitness1, const vector<double>& fitness2);

//implements crowded-comparison operator from Deb et al. 2002
bool crowdedComp(Alignment* aln1, Alignment* aln2);

//precondition: in alns have crowdDist, domCount set.
vector<Alignment*> binSel(RandGenT& prng,
	                      const vector<Alignment*>& in, unsigned int tournSize);

void reportStats(const vector<Alignment*>& in,
                 const vector<fitnessName> fitnessNames, bool verbose,
                 bool alnDiversity);

//returns 0 if the two alignments have no pairs in common, and 1.0
//if the smaller alignment is a perfect subalignment of the other.
//In general, returns number of shared pairs divided by smaller alignment size.
double alnSimilarity(const Alignment* aln1, const Alignment* aln2);