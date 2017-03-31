#pragma once

#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <unordered_set>

#include "blastinfo.h"
#include "Network.h"
#include "goc.h"

class Alignment{
public:	
	//default constructor for efficiency purposes.
	//does NO initialization. Do not use unless you
	//know exactly what you are doing.
	Alignment(){}; //todo: use this in steadystate.cpp
    //simplest constructor creates an arbitrary non-random alignment
	Alignment(const Network* n1, const Network* n2, 
		      const BLASTDict* bit, const GOCDict* goc);
    //load alignment from file
	Alignment(const Network* n1, const Network* n2, string filename,
		      const BLASTDict* bit, const GOCDict* goc);
	//crossover constructor (UPMX)
    Alignment(RandGenT& prng, float cxswappb, 
		             const Alignment& p1,
		             const Alignment& p2,
		             bool total = true);
	void greedyMatch(bool bit);
	void seedExtend(bool bit, bool degdiff);
	void shuf(RandGenT& prng, bool uniformsize,
	          bool smallstart, bool total); //shuffles the alignment to make it completely random
	void mutate(RandGenT& prng, float mutswappb, bool total = true);
	void doSwap(node x, node y);
	void onBit(node x);
	void computeFitness(const vector<fitnessName>& fitnessNames);
	void save(string filename) const;
	double ics() const;
	
	double sumBLAST() const;
    double sumGOC() const;
	double alnSize() const;	
	vector<node> aln;
	vector<node> alnInv; //stores the inverse of aln
	vector<bool> alnMask; //indicates whether the corresponding node
	                      //of V2 is allowed to be aligned to.
	                      //The idea here is that a node in the larger
	                      //net may have no ortholog in the smaller,
	                      //so we might want to give up on aligning it.
						  //This presents some difficulties in that the
		                  //same alignment has many representations.
						  //A node in V2 can be unaligned because it is
	                      //aligned to a dummy node or because its mask
	                      //bit is switched off.
		                  // but if we separate searching through permutations
	                      //from searching through masks, it should be 
	                      //workable.
	
	bool fitnessValid;
	vector<double> fitness; //all fitnesses stored s.t. larger is better.
	vector<double> fitnessNormalized; //stores fitnesses normalized so
	                                  //that crowded comparison works right.
	int actualSize; //number of nodes in net1

	//NSGA-II helper data. Set and modified by NSGA-II functions.
	int domRank; //which front this aln is in. 0 is best, 1 is 2nd best, etc.
	int numThatDominate; //how many others in the population dominate this one.
	vector<Alignment*> dominated;//set of alns this aln dominates.
	double crowdDist;

	//framework for doing delta updates of bitscore sum
	double currBitscore;
	const BLASTDict* bitscores;
	void updateBitscore(node n1, node n2old, node n2new, bool oldMask,
		                bool newMask);

    
    //same stuff for GOC
    double currGOC;
    const GOCDict* gocs;
    void updateGOC(node n1, node n2old, node n2new, bool oldMask,
                   bool newMask);
    
	//stored info version of ICS for fast computation
	//(incrementally update as the alignment changes)
	double fastICSDenominator() const;
	double fastICSNumerator() const;
	double fastICS() const;

	double fastEC() const;
	double fastS3() const;

	//experimental metrics suggested by Rob Patro:
	double icsTimesEC() const;
	double s3Variant() const;
	//pointers to our networks
	const Network* net1;
	const Network* net2;

	//functions for incrementally updating topological fitness function data
	int currConservedCount; //stores current number of conserved edges. Actual number.
	                        //does not double count.
	int conservedCount(node n1, node n2, bool mask, node ignore) const;
	void updateConservedCount(node n1, node n2old, node n2new, bool oldMask,
		                     bool newMask, node ignore);
	void initConservedCount(node n1, node n2, bool mask);

	//tracks how many edges are in the subgraph of net2 induced on the nodes
	//of net2 we have mapped to.
	int currInducedCount;
	int inducedCount(node n2, node ignore) const;
};

