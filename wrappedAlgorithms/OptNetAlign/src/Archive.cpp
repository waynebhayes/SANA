#include "Archive.h"
#include "Network.h"
#include "nsga-ii.h"
#include "Alignment.h"

#include <vector>
#include <set>
#include <algorithm>
#include <random>

//any aln given to Archive can be deleted at any time!
bool Archive::insert(Alignment* aln){

	set<Alignment*> dominatedByAln;

	bool alnIsDominated = false;

	for(auto p : nonDominated){

		if(dominates(aln->fitness,p->fitness)){
			dominatedByAln.insert(p);
		}
		else if(dominates(p->fitness,aln->fitness)){
			alnIsDominated = true;
			break;
		}
	}

	if(alnIsDominated){
		delete aln;
		return false;
	}
	else{
		vector<Alignment*> newNonDominated;
		
		for(auto p : nonDominated){
			if(dominatedByAln.count(p)){
				delete p;
			}
			else{
				newNonDominated.push_back(p);
			}
		}
		newNonDominated.push_back(aln);
		nonDominated = newNonDominated;
		return true;
	}
}

void Archive::shrinkToSize(int size){

	if(size >= nonDominated.size()){
		return;
	}

	normalizeFitnesses(nonDominated);
	setCrowdingDists(nonDominated);

	sort(nonDominated.begin(), nonDominated.end(), 
		[](Alignment* p1, Alignment* p2){
			return p1->crowdDist > p2->crowdDist;
		}
	);

	while(nonDominated.size() > size){
		delete nonDominated.back();
		nonDominated.pop_back();
	}

}