#pragma once

#include "Network.h"
#include "Alignment.h"
#include "nsga-ii.h"

#include <vector>
#include <random>
#include <set>
using namespace std;

/*
Archive maintains a set of non-dominated alignments. Any alignment
in the archive can be deleted at any time, including ones that have
just been inserted. Archive is responsible for memory management of
alignments once they have been inserted.

Therefore, when taking an alignment from the archive, make a copy!

todo: currently assuming Archive lives the entire life of the 
program, so a destructor that cleans up all the alignments is
not needed.
*/

class Archive{
public:
	bool insert(Alignment* aln);
	void shrinkToSize(int size);
	vector<Alignment*> nonDominated;
};