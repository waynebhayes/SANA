#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <utility>
#include <cmath>
#include <iostream>
#include <set>
#include <random>
#include <limits>
#include "Alignment.h"
#include "Network.h"
#include "blastinfo.h"
#include "nsga-ii.h"
#include "goc.h"
#include "localSearch.h"
using namespace std;

bool approxEqual(double x, double y){
	return abs(x-y) < 0.000000001;
}


BOOST_AUTO_TEST_CASE( bitscoreSum_match_1 )
{
    Network net1("../optnetalign/tests/small.net");
    Network net2("../optnetalign/tests/small.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/small.bitScores");
    Alignment a= Alignment(&net1,&net2,"../optnetalign/tests/small.aln",b,nullptr);
    double sumbit = a.sumBLAST();
    BOOST_CHECK(approxEqual(2.0,a.sumBLAST()));
}

BOOST_AUTO_TEST_CASE( bitscoreSum_match_2 )
{
	Network net1("../optnetalign/tests/small.net");
    Network net2("../optnetalign/tests/small.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/small.bitScores");
    Alignment a2 = Alignment(&net1,&net2,"../optnetalign/tests/small2.aln",b,nullptr);
    double sumbit2 = a2.sumBLAST();
    std::cout<<"sumbit2 is "<<sumbit2<<std::endl;
    BOOST_CHECK(approxEqual(sumbit2,0.0));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent ){
	Network net1("../optnetalign/tests/small.net");
    Network net2("../optnetalign/tests/small.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/small.bitScores");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    mt19937 g1(12);
    a2.shuf(g1,false,false,true);
    double sumbit2 = a2.sumBLAST();
    BOOST_CHECK(approxEqual(sumbit2,a2.currBitscore));
}


BOOST_AUTO_TEST_CASE( bitscoreSum_match_3 )
{
	Network net1("../optnetalign/tests/bittest1.net");
    Network net2("../optnetalign/tests/bittest2.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/bittest.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    double sumbit2 = a2.sumBLAST();
    std::cout<<"sumbit2 is "<<sumbit2<<std::endl;
    BOOST_CHECK(approxEqual(sumbit2,11.0+22.0+33.0+44.0));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent_2 ){
	Network net1("../optnetalign/tests/bittest1.net");
    Network net2("../optnetalign/tests/bittest2.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/bittest.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    
    Alignment a3 = Alignment(&net1,&net2,b,nullptr);
    mt19937 g1(12);
    a2.shuf(g1,false,false,false);
    a3.shuf(g1,false,false,false);
    Alignment a4 = Alignment(g1,0.5,a2,a3,false);
    double sumbit2 = a4.sumBLAST();
    cout<<"sumbit2: "<<sumbit2<<endl;
    cout<<"currBitscore: "<<a4.currBitscore<<endl;
    BOOST_CHECK(approxEqual(sumbit2,a4.currBitscore));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent_3 ){
	Network net1("../optnetalign/tests/bittest1.net");
    Network net2("../optnetalign/tests/bittest2.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/bittest.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    a2.currBitscore = a2.sumBLAST();
    mt19937 g1(12);
    a2.mutate(g1,0.25);
    double sumbit2 = a2.sumBLAST();
 	cout<<"sumbit2: "<<sumbit2<<endl;
    cout<<"currBitscore: "<<a2.currBitscore<<endl;   
    BOOST_CHECK(approxEqual(sumbit2,a2.currBitscore));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent_4 ){
	Network net1("../optnetalign/tests/bittest1.net");
    Network net2("../optnetalign/tests/bittest2.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/bittest.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    a2.currBitscore = a2.sumBLAST();
    node temp = a2.aln[1];
    a2.aln[1] = a2.aln[2];
    a2.aln[2] = temp;

    a2.updateBitscore(1, temp, a2.aln[1], true, true);
    a2.updateBitscore(2, a2.aln[1], a2.aln[2], true, true);

    double sumbit2 = a2.sumBLAST();
    BOOST_CHECK(approxEqual(sumbit2,a2.currBitscore));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent_5 ){
	Network net1("../optnetalign/tests/cg1a.net");
    Network net2("../optnetalign/tests/cg1b.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/cg1.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    
    Alignment a3 = Alignment(&net1,&net2,b,nullptr);
    mt19937 g1(12);
    a2.shuf(g1,false,false,false);
    a3.shuf(g1,false,false,false);
    Alignment a4 = Alignment(g1,0.5,a2,a3,false);
    double sumbit2 = a4.sumBLAST();
    cout<<"sumbit2: "<<sumbit2<<endl;
    cout<<"currBitscore: "<<a4.currBitscore<<endl;
    BOOST_CHECK(approxEqual(sumbit2,a4.currBitscore));
}

BOOST_AUTO_TEST_CASE( currBitscore_consistent_6 ){
	Network net1("../optnetalign/tests/cg1a.net");
    Network net2("../optnetalign/tests/cg1b.net");
    BLASTDict* b = loadBLASTInfo(&net1,&net2,
    	                        "../optnetalign/tests/cg1.sim");
    Alignment a2 = Alignment(&net1,&net2,b,nullptr);
    
    Alignment a3 = Alignment(&net1,&net2,b,nullptr);
    mt19937 g1(1244);
    a2.shuf(g1,false,false,true);
    a3.shuf(g1,false,false,true);
    Alignment a4 = Alignment(g1,0.5,a2,a3,true);
    double sumbit2 = a4.sumBLAST();
    cout<<"sumbit2: "<<sumbit2<<endl;
    cout<<"currBitscore: "<<a4.currBitscore<<endl;
    BOOST_CHECK(approxEqual(sumbit2,a4.currBitscore));
}

BOOST_AUTO_TEST_CASE( save_load_inverses )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln1(&net1,&net2,"../optnetalign/tests/lccstest.aln",nullptr,nullptr);
	aln1.save("../optnetalign/tests/testSavecpp.aln");
	Alignment aln2(&net1,&net2,"../optnetalign/tests/testSavecpp.aln",nullptr,nullptr);

	for(int i = 0; i < net1.nodeToNodeName.size(); i++){
		BOOST_CHECK(aln1.aln[i] == aln2.aln[i]);
	}
}

BOOST_AUTO_TEST_CASE( rand_aln_consistent )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	set<node> v2Set;
	//check that alignment is valid permutation.
	for(int i = 0; i < aln1.aln.size(); i++){
		v2Set.insert(aln1.aln[i]);
		BOOST_CHECK(aln1.aln[i] >= 0 && 
			        aln1.aln[i] < net2.nodeToNodeName.size());
	}
	BOOST_CHECK(v2Set.size() == aln1.aln.size());
}

BOOST_AUTO_TEST_CASE( load_aln_consistent )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	set<node> v2Set;
	vector<int> counts(aln1.aln.size(),0);

	for(int i = 0; i < aln1.aln.size(); i++){
		BOOST_CHECK(aln1.aln[i] >= 0 &&
			        aln1.aln[i] < net2.nodeToNodeName.size());
		if(v2Set.count(aln1.aln.at(i))){
			cout<<"Already aligned! aln1.aln["<<i<<"]: "<<net2.nodeToNodeName[aln1.aln.at(i)]<<endl;
		}
		v2Set.insert(aln1.aln.at(i));
		if(counts.at(aln1.aln.at(i)) > 1){
			cout<<"node "<<aln1.aln.at(i)<<" appears twice!"<<endl;
		}
		counts[aln1.aln.at(i)]++;
	}

	for(int i = 0; i < aln1.aln.size(); i++){
		for(int j=i+1; j < aln1.aln.size(); j++){
			if(i == j){
				cout<<"found same node aligned to twice."<<endl;
				cout<<"node "<<net2.nodeToNodeName[i]<<endl;
			}
		}
	}

	for(int i = 0; i < aln1.aln.size(); i++){
		if(!v2Set.count(aln1.aln[i])){
			cout<<"missing node "<<net2.nodeToNodeName[aln1.aln[i]]<<endl;
		}
	}

	aln1.save("problem.aln");
	BOOST_CHECK(v2Set.size() == aln1.aln.size());
}

BOOST_AUTO_TEST_CASE( ics_match_1 )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest.aln",nullptr,nullptr);
	double ics = aln.ics();
	BOOST_CHECK(approxEqual(ics,0.8));
}

BOOST_AUTO_TEST_CASE( fast_ics_match_1 )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest.aln",nullptr,nullptr);
	double ics = aln.fastICS();
	BOOST_CHECK(approxEqual(ics,0.8));
}

BOOST_AUTO_TEST_CASE( ics_match_2 )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest2.aln",nullptr,nullptr);
	double ics = aln.ics();
	BOOST_CHECK(approxEqual(ics,0.0));
}

BOOST_AUTO_TEST_CASE( fast_ics_match_2 )
{
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest2.aln",nullptr,nullptr);
	double ics = aln.fastICS();
	BOOST_CHECK(approxEqual(ics,0.0));
}

BOOST_AUTO_TEST_CASE( ics_match_3 )
{
	Network net1("../optnetalign/tests/newmetrica.net");
	Network net2("../optnetalign/tests/newmetricb.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/newmetric.aln",nullptr,nullptr);
	double ics = aln.ics();
	BOOST_CHECK(approxEqual(ics,0.666666666666666));
}

BOOST_AUTO_TEST_CASE( fast_ics_match_3 )
{
	Network net1("../optnetalign/tests/newmetrica.net");
	Network net2("../optnetalign/tests/newmetricb.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/newmetric.aln",nullptr,nullptr);
	double ics = aln.fastICS();
	BOOST_CHECK(approxEqual(ics,0.666666666666666));
}

//todo: the following 2 tests are broken while self-loops are ignored.

BOOST_AUTO_TEST_CASE( ics_match_4 )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	double ics = aln.ics();
	BOOST_CHECK(approxEqual(ics,0.8253323173313013));
}

BOOST_AUTO_TEST_CASE( fast_ics_match_4 )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	double ics = aln.fastICS();
	BOOST_CHECK(approxEqual(ics,0.8253323173313013));
}

BOOST_AUTO_TEST_CASE( ics_match_5 )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1partial.aln",nullptr,nullptr);
	double ics = aln.ics();
	BOOST_CHECK(approxEqual(ics,0.920399546905571));
}

BOOST_AUTO_TEST_CASE( fast_ics_match_5 )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1partial.aln",nullptr,nullptr);
	double ics = aln.fastICS();
	BOOST_CHECK(approxEqual(ics,0.920399546905571));
}

BOOST_AUTO_TEST_CASE( consistent_after_mutate )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	mt19937 g1(12);
	aln.mutate(g1, 0.3);
	set<node> v2Set;
	for(int i = 0; i < aln.aln.size(); i++){
		v2Set.insert(aln.aln[i]);
		BOOST_CHECK(aln.aln[i] >=0 && 
			        aln.aln[i] < net2.nodeToNodeName.size());
	}

	BOOST_CHECK(v2Set.size() == aln.aln.size());

}

BOOST_AUTO_TEST_CASE( consistent_after_crossover )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	Alignment aln2(&net1,&net2,"../optnetalign/tests/cg1partial.aln",nullptr,nullptr);
	mt19937 g1(12);
	Alignment child(g1, 0.3, aln, aln2);
	set<node> v2Set;
	for(int i = 0; i < aln.aln.size(); i++){
		v2Set.insert(aln.aln[i]);
		BOOST_CHECK(aln.aln[i] >=0 && 
			        aln.aln[i] < net2.nodeToNodeName.size());
	}

	BOOST_CHECK(v2Set.size() == aln.aln.size());

}

BOOST_AUTO_TEST_CASE( nondominated_sort )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	aln1.fitness.push_back(0.7);
	aln1.fitness.push_back(0.8);
	Alignment aln2(&net1,&net2,nullptr,nullptr);
	aln2.fitness.push_back(0.75);
	aln2.fitness.push_back(0.75);
	Alignment aln3(&net1,&net2,nullptr,nullptr);
	aln3.fitness.push_back(0.8);
	aln3.fitness.push_back(0.7);
	Alignment aln4(&net1,&net2,nullptr,nullptr);
	aln4.fitness.push_back(0.4);
	aln4.fitness.push_back(0.6);
	Alignment aln5(&net1,&net2,nullptr,nullptr);
	aln5.fitness.push_back(0.5);
	aln5.fitness.push_back(0.5);
	Alignment aln6(&net1,&net2,nullptr,nullptr);
	aln6.fitness.push_back(0.6);
	aln6.fitness.push_back(0.4);
	vector<Alignment*> toSort;
	toSort.push_back(&aln3);
	toSort.push_back(&aln2);
	toSort.push_back(&aln4);
	toSort.push_back(&aln1);
	toSort.push_back(&aln6);
	toSort.push_back(&aln5);
	vector<vector<Alignment*> > fronts = nonDominatedSort(toSort);
	for(int i = 0; i < fronts.size(); i++){
		cout<<"FRONT "<<i<<endl;
		cout<<"------------"<<endl;
		for(int j = 0; j <fronts[i].size(); j++){
			for(int k = 0; k < fronts[i][j]->fitness.size(); k++){
				cout<<fronts[i][j]->fitness[k]<<" "<<endl;
			}
			cout<<endl;
		}
	}
	//ensure we have the right number of fronts
	BOOST_CHECK(fronts.size()==2);
	//and they are the right sizes
	BOOST_CHECK(fronts[0].size() == 3);
	BOOST_CHECK(fronts[1].size() == 3);
	//and they each contain the fitnesses we think they should
	for(int i = 0; i < 3; i++){
		BOOST_CHECK(fronts[0][i]->fitness[0] > 0.6);
		BOOST_CHECK(fronts[0][i]->fitness[1] > 0.6);
	}
	for(int i = 0; i < 3; i++){
		BOOST_CHECK(fronts[1][i]->fitness[0] < 0.7);
		BOOST_CHECK(fronts[1][i]->fitness[1] < 0.7);
	}
}

BOOST_AUTO_TEST_CASE( crowding_dist_assignment )
{
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	aln1.fitness.push_back(0.6);
	aln1.fitness.push_back(0.9);
	Alignment aln2(&net1,&net2,nullptr,nullptr);
	aln2.fitness.push_back(0.7);
	aln2.fitness.push_back(0.8);
	Alignment aln3(&net1,&net2,nullptr,nullptr);
	aln3.fitness.push_back(0.75);
	aln3.fitness.push_back(0.75);
	Alignment aln4(&net1,&net2,nullptr,nullptr);
	aln4.fitness.push_back(0.8);
	aln4.fitness.push_back(0.7);
	Alignment aln5(&net1,&net2,nullptr,nullptr);
	aln5.fitness.push_back(0.9);
	aln5.fitness.push_back(0.6);
	vector<Alignment*> front;
	front.push_back(&aln1);
	front.push_back(&aln2);
	front.push_back(&aln3);
	front.push_back(&aln4);
	front.push_back(&aln5);
	normalizeFitnesses(front);
	setCrowdingDists(front);
	BOOST_CHECK(approxEqual(1.0,front[1]->crowdDist));
	BOOST_CHECK(approxEqual(0.666666666666666,front[2]->crowdDist));
	BOOST_CHECK(approxEqual(1.0,front[3]->crowdDist));
}

BOOST_AUTO_TEST_CASE( adjacency_list_correct ){
	Network net1("../optnetalign/tests/lccstest1.net");
	BOOST_CHECK(net1.degree(net1.nodeNameToNode.at("1")) == 1);
	BOOST_CHECK(net1.degree(net1.nodeNameToNode.at("2")) == 2);
	BOOST_CHECK(net1.degree(net1.nodeNameToNode.at("3")) == 2);
	BOOST_CHECK(net1.degree(net1.nodeNameToNode.at("4")) == 3);
}

BOOST_AUTO_TEST_CASE( adjList_correct_selfloop ){
	Network net1("../optnetalign/tests/selflooptest.net");
	BOOST_CHECK(net1.degree(net1.nodeNameToNode.at("1")) == 2);
}

BOOST_AUTO_TEST_CASE( adjList_adjMatrix_consistent ){
	Network net1("../optnetalign/tests/cg1a.net");

	cout<<"BEGIN adjList_adjMatrix_consistent"<<endl;

	cout<<"Checking that all adjList elems are in adjMatrix"<<endl;
	for(int i = 0; i < net1.adjList.size(); i++){
		for(int j = 0; j < net1.adjList.at(i).size(); j++){
			BOOST_CHECK(net1.adjMatrix[i][net1.adjList.at(i).at(j)]);
			BOOST_CHECK(net1.adjMatrix[net1.adjList.at(i).at(j)][i]);
		}
	}

	cout<<"Checking that all adjMatrix neighbors are in adjList"<<endl;
	for(int i = 0; i < net1.adjMatrix.size(); i++){
		for(int j = 0; j < net1.adjMatrix.at(i).size(); j++){
			if(net1.adjMatrix.at(j).at(i)){
				bool found = false;
				for(int k = 0; k < net1.adjList.at(i).size(); k++){
					if(net1.adjList.at(i).at(k) == j){
						found = true;
					}
				}
				BOOST_CHECK(found);

				bool found2 = false;
				for(int k = 0; k < net1.adjList.at(j).size(); k++){
					if(net1.adjList.at(j).at(k) == i){
						found2 = true;
					}
				}
				BOOST_CHECK(found2);
			}
			else{ //if not aligned in matrix, assure not aligned in list
				bool notFound = true;
				for(int k = 0; k < net1.adjList.at(i).size(); k++){
					if(net1.adjList.at(i).at(k) == j){
						notFound = false;
					}
				}
				BOOST_CHECK(notFound);

				bool notFound2 = true;
				for(int k = 0; k < net1.adjList.at(j).size(); k++){
					if(net1.adjList.at(j).at(k) == i){
						notFound2 = false;
					}
				}
				BOOST_CHECK(notFound2);
			}
		}
	}
}


BOOST_AUTO_TEST_CASE( fast_ics_works_total_crossover ){
	cout<<endl<<"BEGIN fast_ics_works_total_crossover"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	Alignment aln2(&net1,&net2,nullptr,nullptr);
	mt19937 g1(12);
	aln1.shuf(g1,false,false,true);
	aln2.shuf(g1,false,false,true);
	Alignment child(g1,0.2,aln1,aln2,true);
	double ics = child.ics();
	double fastICS = child.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));
}


BOOST_AUTO_TEST_CASE( fast_ics_works_partial_crossover ){
	cout<<endl<<"BEGIN fast_ics_works_partial_crossover"<<endl;
	for(int i = 0; i < 1; i++){
		cout<<"-------------------"<<endl;
		Network net1("../optnetalign/tests/selflooptest.net");
		Network net2("../optnetalign/tests/selflooptest.net");
		Alignment aln1(&net1,&net2,nullptr,nullptr);
		Alignment aln2(&net1,&net2,nullptr,nullptr);
		mt19937 g1(i);
		aln1.shuf(g1,false,false,false);
		aln2.shuf(g1,false,false,false);
		cout<<"aln1 is :"<<endl;
		for(int i = 0; i < aln1.aln.size(); i++){
			cout<<net1.nodeToNodeName.at(i)<<" "
				<<net2.nodeToNodeName.at(aln1.aln[i])
				<<" "<<aln1.alnMask[i]<<endl;
		}
		cout<<"aln1.currConservedCount is: "<<aln1.currConservedCount<<endl;

		cout<<"aln2 is :"<<endl;
		for(int i = 0; i < aln1.aln.size(); i++){
			cout<<net1.nodeToNodeName.at(i)<<" "
				<<net2.nodeToNodeName.at(aln2.aln[i])
				<<" "<<aln2.alnMask[i]<<endl;
		}
		cout<<"aln2.currConservedCount is: "<<aln2.currConservedCount<<endl;

		Alignment child(g1,0.2,aln1,aln2,false);

		cout<<"child is :"<<endl;
		for(int i = 0; i < aln1.aln.size(); i++){
			cout<<net1.nodeToNodeName.at(i)<<" "
				<<net2.nodeToNodeName.at(child.aln[i])
				<<" "<<child.alnMask[i]<<endl;
		}
		cout<<"child.currConservedCount is: "<<child.currConservedCount<<endl;
		double ics = child.ics();
		double fastICS = child.fastICS();
		cout<<"ICS is "<<ics<<endl;
		cout<<"Fast ICS is "<<fastICS<<endl;
		BOOST_CHECK(approxEqual(ics,fastICS));
	}
}


BOOST_AUTO_TEST_CASE( fast_ics_works_total_mutation ){
	cout<<endl<<"BEGIN fast_ics_works_total_mutation"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(12);
	aln1.shuf(g1,false,false,true);
	aln1.mutate(g1,0.1,true);
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}

BOOST_AUTO_TEST_CASE( fast_ics_works_total_repeat_mutation ){
	cout<<endl<<"BEGIN fast_ics_works_total_repeat_mutation"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(12);
	aln1.shuf(g1,false,false,true);
	for(int i = 0; i < 100; i++){
		aln1.mutate(g1,0.1,true);
	}
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}

BOOST_AUTO_TEST_CASE( fast_ics_works_partial_mutation ){
	cout<<endl<<"BEGIN fast_ics_works_partial_mutation"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(4);
	aln1.shuf(g1,false,false,false);
	
	aln1.mutate(g1,0.1,false);
	
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}

BOOST_AUTO_TEST_CASE( fast_ics_works_partial_repeat_mutation ){
	cout<<endl<<"BEGIN fast_ics_works_total_repeat_mutation"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(12);
	aln1.shuf(g1,false,false,false);
	for(int i = 0; i < 100; i++){
		aln1.mutate(g1,0.1,false);
	}
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}


BOOST_AUTO_TEST_CASE( fast_ics_works_after_init_total ){
	cout<<endl<<"BEGIN fast_ics_works_after_init_total"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(12);
	aln1.shuf(g1,false,false,true);
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}


BOOST_AUTO_TEST_CASE( fast_ics_works_after_init_partial ){
	cout<<endl<<"BEGIN fast_ics_works_after_init_partial"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(123);
	aln1.shuf(g1,false,false,false);
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}

BOOST_AUTO_TEST_CASE( do_swap_consistent_ics ){
	cout<<endl<<"BEGIN do_swap_consistent_ics"<<endl;
	Network net1("../optnetalign/tests/selflooptest.net");
	Network net2("../optnetalign/tests/selflooptest.net");
	Alignment aln1(&net1,&net2,nullptr,nullptr);
	mt19937 g1(124);
	aln1.shuf(g1,false,false,true);
	
	
	
	double icsBefore = aln1.ics();
	double fastICSBefore = aln1.fastICS();
	cout<<"icsBefore: "<<icsBefore<<endl;
	cout<<"fastICSBefore: "<<fastICSBefore<<endl;
	BOOST_CHECK(approxEqual(icsBefore,fastICSBefore));
	auto dist = uniform_int_distribution<int>(0,aln1.aln.size()-1);
	node x = dist(g1);
	node y = dist(g1);
	//cout<<"x is "<<net1.nodeToNodeName.at(x)<<endl;
	//cout<<"y is "<<net1.nodeToNodeName.at(y)<<endl;
	aln1.doSwap(x,y);
	
	double ics = aln1.ics();
	double fastICS = aln1.fastICS();
	cout<<"ICS is "<<ics<<endl;
	cout<<"Fast ICS is "<<fastICS<<endl;
	BOOST_CHECK(approxEqual(ics,fastICS));	
}

BOOST_AUTO_TEST_CASE( greedy_match_aln_consistent ){
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	BLASTDict* b = loadBLASTInfo(&net1,&net2,"../optnetalign/tests/cg1.sim");
	Alignment aln(&net1,&net2,b,nullptr);

	//check that the aln is a permutation
	unordered_set<node> alnv2;

	for(int i = 0; i < aln.aln.size(); i++){
		alnv2.insert(aln.aln[i]);
	}

	BOOST_CHECK_EQUAL(alnv2.size(),aln.aln.size());
}

BOOST_AUTO_TEST_CASE( self_similarity_is_one){
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	BLASTDict* b = loadBLASTInfo(&net1,&net2,"../optnetalign/tests/cg1.sim");
	Alignment aln(&net1,&net2,"../optnetalign/tests/cg1.aln",b,nullptr);
	BOOST_CHECK(approxEqual(alnSimilarity(&aln,&aln),1.0));

}

BOOST_AUTO_TEST_CASE(similarity_check){
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln1(&net1,&net2,"../optnetalign/tests/cg1.aln",nullptr,nullptr);
	Alignment aln2(&net1,&net2,"../optnetalign/tests/cg1partial.aln",nullptr,nullptr);
	double sim = alnSimilarity(&aln1,&aln2);
	BOOST_CHECK(approxEqual(sim,0.8891585761));
}

BOOST_AUTO_TEST_CASE(goc_test){
    Network net1("../optnetalign/tests/small.net");
    Network net2 = net1;
    string fp = "../optnetalign/tests/small.annos";
    cout<<"Loading gocs"<<endl;
    GOCDict* gocs = loadGOC(&net1,&net2,fp,fp);
    cout<<"Calling aln constructor"<<endl; 
    Alignment aln(&net1,&net2,nullptr,gocs);
    mt19937 g(1);
    cout<<"calling shuf"<<endl;
    aln.shuf(g,false,false,true);
    BOOST_CHECK(approxEqual(aln.sumGOC(),2.0));
}

BOOST_AUTO_TEST_CASE(currGOC_after_doSwap){
    Network net1("../optnetalign/tests/small.net");
    Network net2 = net1;
    string fp = "../optnetalign/tests/small.annos";
    cout<<"Loading gocs"<<endl;
    GOCDict* gocs = loadGOC(&net1,&net2,fp,fp);
    cout<<"Calling aln constructor"<<endl; 
    Alignment aln(&net1,&net2,nullptr,gocs);
    mt19937 g(1);
    cout<<"calling shuf"<<endl;
    aln.shuf(g,false,false,true);
    aln.doSwap(0,1);
    BOOST_CHECK(approxEqual(aln.currGOC,2.0));
}

BOOST_AUTO_TEST_CASE( GOC_consistent ){
	cout<<"BEGIN GOC_consistent"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest1.net");
	GOCDict* gocs = loadGOC(&net1,&net2,
		                   "../optnetalign/tests/lccstest1.annos",
		                   "../optnetalign/tests/lccstest1.annos");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest1iso.aln",nullptr,gocs);
	BOOST_CHECK(approxEqual(4.0, aln.sumGOC()));
	BOOST_CHECK(approxEqual(4.0, aln.currGOC));
	aln.doSwap(0,1);
	BOOST_CHECK(approxEqual(2.666666666666, aln.currGOC));
	BOOST_CHECK(approxEqual(2.666666666666, aln.sumGOC()));
}

BOOST_AUTO_TEST_CASE( GOC_after_cx ){
	cout<<"BEGIN GOC_after_cx"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest1.net");
	GOCDict* gocs = loadGOC(&net1, &net2,
		                   "../optnetalign/tests/lccstest1.annos",
		                   "../optnetalign/tests/lccstest1.annos");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest1iso.aln",nullptr,gocs);
	Alignment aln2(&net1,&net2,"../optnetalign/tests/lccstest1other.aln",nullptr,gocs);
	vector<fitnessName> fitnessNames;
	fitnessNames.push_back(GOCFit);
	aln.computeFitness(fitnessNames);
	aln2.computeFitness(fitnessNames);
	mt19937 g(1);
	Alignment alnChild(g,0.5,aln,aln2,true);
	alnChild.computeFitness(fitnessNames);
	cout<<"Child alignment is: "<<endl;
	for(int i = 0; i < alnChild.aln.size(); i++){
		cout<<net1.nodeToNodeName.at(i)<<" "<<net1.nodeToNodeName.at(alnChild.aln[i])<<endl;
	}
	BOOST_CHECK(approxEqual(aln.currGOC,aln.sumGOC()));
}

BOOST_AUTO_TEST_CASE( GOC_after_mut ){
	cout<<"BEGIN GOC_after_mut"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest1.net");
	GOCDict* gocs = loadGOC(&net1, &net2,
		                   "../optnetalign/tests/lccstest1.annos",
		                   "../optnetalign/tests/lccstest1.annos");
	Alignment aln(&net1,&net2,"../optnetalign/tests/lccstest1iso.aln",nullptr,gocs);
	vector<fitnessName> fitnessNames;
	fitnessNames.push_back(GOCFit);
	aln.computeFitness(fitnessNames);
	mt19937 g(1);
	aln.mutate(g,0.5,true);
	cout<<"Mutated alignment is: "<<endl;
	for(int i = 0; i < aln.aln.size(); i++){
		cout<<net1.nodeToNodeName.at(i)<<" "<<net1.nodeToNodeName.at(aln.aln[i])<<endl;
	}
	BOOST_CHECK(approxEqual(aln.currGOC,aln.sumGOC()));
}

BOOST_AUTO_TEST_CASE( GOC_after_mut_partial ){
	cout<<"BEGIN GOC_after_mut_partial"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest1.net");
	GOCDict* gocs = loadGOC(&net1, &net2,
		                   "../optnetalign/tests/lccstest1.annos",
		                   "../optnetalign/tests/lccstest1.annos");
	Alignment aln(&net1,&net2,nullptr,gocs);
	vector<fitnessName> fitnessNames;
	fitnessNames.push_back(GOCFit);
	aln.computeFitness(fitnessNames);
	mt19937 g(1);
	aln.shuf(g,false,false,false);
	aln.computeFitness(fitnessNames);
	aln.mutate(g,0.5,true);
	BOOST_CHECK(approxEqual(aln.currGOC,aln.sumGOC()));	
}

BOOST_AUTO_TEST_CASE( GOC_after_cx_partial ){
	cout<<"BEGIN GOC_after_cx_partial"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest1.net");
	GOCDict* gocs = loadGOC(&net1, &net2,
		                   "../optnetalign/tests/lccstest1.annos",
		                   "../optnetalign/tests/lccstest1.annos");
	Alignment aln(&net1,&net2,nullptr,gocs);
	Alignment aln2(&net1,&net2,nullptr,gocs);
	vector<fitnessName> fitnessNames;
	fitnessNames.push_back(GOCFit);
	aln.computeFitness(fitnessNames);
	aln2.computeFitness(fitnessNames);
	mt19937 g(1);
	aln.shuf(g,false,false,false);
	aln2.shuf(g,false,false,false);
	aln.computeFitness(fitnessNames);
	aln2.computeFitness(fitnessNames);
	Alignment child(g,0.5,aln,aln2,false);
	BOOST_CHECK(approxEqual(child.currGOC,child.sumGOC()));	
}

BOOST_AUTO_TEST_CASE( alnInv_consistent_after_constructors ){
	cout<<"BEGIN alnInv_consistent_after_constructors"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");

	Alignment arbConstructor(&net1,&net2,nullptr,nullptr);
	for(int i = 0; i < arbConstructor.aln.size(); i++){
		BOOST_CHECK_EQUAL(arbConstructor.aln[i],
			              arbConstructor.alnInv[arbConstructor.aln[i]]);
	}

	Alignment arb2 = arbConstructor;
	for(int seed = 0; seed < 10000; seed++){
		RandGenT g(seed);

		Alignment offspring(g, 0.5, arbConstructor, arb2, true);
		for(int i = 0; i < offspring.aln.size(); i++){
			BOOST_CHECK_EQUAL(i, offspring.alnInv[offspring.aln[i]]);
		}
	}
	Alignment loaded(&net1,&net2,"../optnetalign/tests/cg1.aln", nullptr, nullptr);
	for(int i = 0; i < loaded.aln.size(); i++){
		BOOST_CHECK_EQUAL(i, loaded.alnInv[loaded.aln[i]]);
	}
}

BOOST_AUTO_TEST_CASE( alnInv_consistent_after_seeding ){
	cout<<"BEGIN alnInv_consistent_after_seeding"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	BLASTDict* bits = loadBLASTInfo(&net1, &net2, "../optnetalign/tests/cg1.sim");
	Alignment aln(&net1,&net2,bits,nullptr);

	aln.greedyMatch(true);

	for(int i = 0; i < aln.aln.size(); i++){
		BOOST_CHECK_EQUAL(i, aln.alnInv[aln.aln[i]]);
	}

	BOOST_CHECK_EQUAL(aln.aln.size(), aln.alnInv.size());
}

BOOST_AUTO_TEST_CASE( alnInv_consistent_after_shuffle ){
	cout<<"BEGIN alnInv_consistent_after_shuffle"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");

	Alignment aln(&net1, &net2, nullptr, nullptr);

	RandGenT g(1);

	aln.shuf(g, false, false, true);

	for(int i = 0; i < aln.aln.size(); i++){
		BOOST_CHECK_EQUAL(i, aln.alnInv[aln.aln[i]]);
	}

	BOOST_CHECK_EQUAL(aln.aln.size(), aln.alnInv.size());
}

BOOST_AUTO_TEST_CASE( alnInv_consistent_after_mutate ){
	cout<<"BEGIN alnInv_consistent_after_mutate"<<endl;
	Network net1("../optnetalign/tests/selflooptest.net");
	Network net2("../optnetalign/tests/selflooptest.net");

	Alignment aln(&net1, &net2, nullptr, nullptr);

	for(int seed = 0; seed < 100; seed++){
		RandGenT g(seed); //fails on 98

		aln.shuf(g, false, false, true);
		/*
		cout<<"BEFORE MUTATE"<<endl;
		cout<<"aln is: "<<endl;
		for(int i = 0; i < aln.aln.size(); i++){
			cout<<net1.nodeToNodeName.at(i)<<' '<<net2.nodeToNodeName.at(aln.aln[i])<<endl;
		}

		cout<<"alnInv is:"<<endl;
		for(int i = 0; i < aln.alnInv.size(); i++){
			cout<<net2.nodeToNodeName.at(i)<<" "<<net1.nodeToNodeName.at(aln.alnInv[i])<<endl;
		}
		*/
		aln.mutate(g, 0.05, true);
		/*
		cout<<"AFTER MUTATE"<<endl;
		cout<<"aln is: "<<endl;
		for(int i = 0; i < aln.aln.size(); i++){
			cout<<net1.nodeToNodeName.at(i)<<' '<<net2.nodeToNodeName.at(aln.aln[i])<<endl;
		}

		cout<<"alnInv is: "<<endl;
		for(int i = 0; i < aln.alnInv.size(); i++){
			cout<<net2.nodeToNodeName.at(i)<<' '<<net1.nodeToNodeName.at(aln.alnInv[i])<<endl;
		}
		*/
		for(int i = 0; i < aln.aln.size(); i++){
			BOOST_CHECK_EQUAL(i, aln.alnInv[aln.aln[i]]);
		}
		//cout<<"seed is "<<seed<<endl;
		BOOST_CHECK_EQUAL(aln.aln.size(), aln.alnInv.size());
	}
}


BOOST_AUTO_TEST_CASE( alnInv_consistent_after_doSwap ){
	
	cout<<"BEGIN alnInv_consistent_after_doSwap"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");

	Alignment aln(&net1, &net2, nullptr, nullptr);

	for(int g = 0; g < 5000; g++){
		
		RandGenT gen(g);
		auto dist = uniform_int_distribution<int>(0,aln.aln.size()-1);
		node x = dist(gen);
		node y = x;
		while(y == x){
			y = dist(gen);
		}

		aln.doSwap(x,y);
		
		for(int i = 0; i < aln.aln.size(); i++){
			BOOST_CHECK_EQUAL(i, aln.alnInv[aln.aln[i]]);
		}
		
	}
	
}

BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_construction ){
	cout<<"BEGIN currInducedCount_consistent_after_construction"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	BLASTDict* bits = loadBLASTInfo(&net1,&net2, "../optnetalign/tests/cg1.sim");
	Alignment aln(&net1, &net2, nullptr, nullptr);

	BOOST_CHECK(approxEqual(aln.fastICSDenominator(), (double)aln.currInducedCount));

	RandGenT g(12);

	aln.shuf(g,false,false,true);

	BOOST_CHECK(approxEqual(aln.fastICSDenominator(), (double)aln.currInducedCount));

	Alignment aln2(&net1, &net2, "../optnetalign/tests/cg1.aln",
				   nullptr, nullptr);

	BOOST_CHECK(approxEqual(aln2.fastICSDenominator(),(double)aln2.currInducedCount));

	Alignment aln3(&net1, &net2, bits, nullptr);
	aln3.greedyMatch(true);
	BOOST_CHECK(approxEqual(aln3.fastICSDenominator(),(double)aln3.currInducedCount));
}


BOOST_AUTO_TEST_CASE( inducedCount_correct ){
	//note: this test only works on nets with no self-loops
	cout<<"BEGIN inducedCount_correct"<<endl;
	Network net1("../optnetalign/tests/lccstest1.net");
	Network net2("../optnetalign/tests/lccstest2.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);
	
	for(int seed = 0; seed < 1000; seed++){
		RandGenT g(seed);
		aln.shuf(g,false,false,true);

		/*
		cout<<"aln is: "<<endl;
		for(int i = 0; i < aln.actualSize; i++){
			cout<<net1.nodeToNodeName.at(i)<<' '<<net2.nodeToNodeName.at(aln.aln[i])<<endl;
		}

		cout<<"alnInv is: "<<endl;
		for(int i = 0; i < aln.alnInv.size(); i++){
			cout<<net2.nodeToNodeName.at(i)<<' '<<(net1.nodeToNodeName.count(aln.alnInv[i])
				                                    ? net1.nodeToNodeName.at(aln.alnInv[i])
				                                    : "unmapped")<<endl;
		}
		*/
		int sum = 0;
		for(int i = 0; i < aln.alnInv.size(); i++){
			//cout<<"calling inducedCount for "<<net2.nodeToNodeName.at(i)<<endl;
			sum += aln.inducedCount(i,-1);
		}

		BOOST_CHECK(approxEqual(aln.fastICSDenominator(), ((double)sum)/2.0));
	}
}

BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_doSwap ){
	cout<<"BEGIN currInducedCount_consistent_after_doSwap"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);

	for(int seed = 0; seed < 1000; seed++){
		RandGenT g(seed);
		aln.shuf(g,false,false,true);

		uniform_int_distribution<int> dist(0,aln.aln.size()-1);

		node x = dist(g);
		node y = x;
		while(y == x){
			y = dist(g);
		}

		aln.doSwap(x,y);

		BOOST_CHECK(approxEqual(aln.fastICSDenominator(), (double)aln.currInducedCount));
	}

}


BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_mutate ){
	cout<<"BEGIN currInducedCount_consistent_after_mutate"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);
	RandGenT g(12);
	aln.shuf(g,false,false,true);

	for(int i = 0; i < 1000; i++){
		aln.mutate(g,0.25,true);
		BOOST_CHECK(approxEqual(aln.fastICSDenominator(),(double)aln.currInducedCount));
	}

}

BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_crossover ){
	cout<<"BEGIN currInducedCount_consistent_after_crossover"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);
	Alignment aln2(&net1,&net2,nullptr,nullptr);
	for(int seed = 0; seed < 1000; seed++){
		RandGenT g(seed);
		aln.shuf(g,false,false,true);
		aln2.shuf(g,false,false,true);

		Alignment child(g, 0.5, aln, aln2, true);

		BOOST_CHECK(approxEqual(aln.fastICSDenominator(), (double)aln.currInducedCount));
	}
}

BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_mutate_partial ){
	cout<<"BEGIN currInducedCount_consistent_after_mutate_partial"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);
	RandGenT g(12);
	aln.shuf(g,false,false,false);

	for(int i = 0; i < 1000; i++){
		aln.mutate(g,0.25,false);
		BOOST_CHECK(approxEqual(aln.fastICSDenominator(),(double)aln.currInducedCount));
	}

}

BOOST_AUTO_TEST_CASE( currInducedCount_consistent_after_crossover_partial ){
	cout<<"BEGIN currInducedCount_consistent_after_crossover_partial"<<endl;
	Network net1("../optnetalign/tests/cg1a.net");
	Network net2("../optnetalign/tests/cg1b.net");
	Alignment aln(&net1,&net2,nullptr,nullptr);
	Alignment aln2(&net1,&net2,nullptr,nullptr);
	for(int seed = 0; seed < 1000; seed++){
		RandGenT g(seed);
		aln.shuf(g,false,false,false);
		aln2.shuf(g,false,false,false);

		Alignment child(g, 0.5, aln, aln2, false);

		BOOST_CHECK(approxEqual(aln.fastICSDenominator(), (double)aln.currInducedCount));
	}
}

BOOST_AUTO_TEST_CASE( velTracker_reports_avg ){
	cout<<"BEGIN velTracker_reports_avg"<<endl;
	VelocityTracker velTracker;

	vector<double> fakeDelta1 = {1.0, 2.0};
	vector<double> fakeDelta2 = {2.0, 1.0};
	vector<double> fakeDelta3 = {1.0, 2.0};

	velTracker.reportDelta(fakeDelta1);
	velTracker.reportDelta(fakeDelta2);
	velTracker.reportDelta(fakeDelta3);

	cout<<"reported deltas"<<endl;

	vector<double> averaged = velTracker.getRecentVel();

	BOOST_CHECK(approxEqual(1.33333333333, averaged[0]));
	BOOST_CHECK(approxEqual(1.66666666666, averaged[1]));
}

BOOST_AUTO_TEST_CASE( velTracker_rolls_over_properly ){
	cout<<"BEGIN velTracker_rolls_over_properly"<<endl;

	VelocityTracker velTracker; 

	for(int i = 0; i < 60; i++){
		vector<double> fakeDelta = {1.0,1.0};
		velTracker.reportDelta(fakeDelta);
	}

	vector<double> averaged = velTracker.getRecentVel();
	BOOST_CHECK(approxEqual(1.0, averaged[0]));
	BOOST_CHECK(approxEqual(1.0, averaged[1]));

}

BOOST_AUTO_TEST_CASE( velTracker_rolls_over_properly2 ){
	cout<<"BEGIN velTracker_rolls_over_properly2"<<endl;

	VelocityTracker velTracker;

	for(int i = 0; i < 50; i++){
		vector<double> fakeDelta = {1.0};
		velTracker.reportDelta(fakeDelta);
	}

	double sum = 0;
	for(int i = 0; i < 50; i++){
		vector<double> fakeDelta;
		fakeDelta.push_back(double(i));
		velTracker.reportDelta(fakeDelta);
		sum += double(i);
	}

	sum /= 50.0;

	vector<double> averaged = velTracker.getRecentVel();
	cout<<"my avg "<<sum<<endl;
	cout<<"velTracker avg "<<averaged[0]<<endl;
	BOOST_CHECK(approxEqual(sum, averaged[0]));
}

/*
BOOST_AUTO_TEST_CASE( GOC_correct_after_load ){
    cout<<"BEGIN GOC_correct_after_load"<<endl;
    Network net1("../optnetalign/tests/dmela.net");
    Network net2("../optnetalign/tests/hsapi.net");
    GOCDict gocs = loadGOC(&net1,&net2,
                           "../optnetalign/tests/dmela.annos",
                           "../optnetalign/tests/hsapi.annos");
    
    Alignment aln(&net1,&net2,"../optnetalign/tests/dm-hs-netal.aln",nullptr,&gocs);
    cout<<"sum of GOC is "<<aln.sumGOC()<<endl;
    BOOST_CHECK(approxEqual(39.7137,aln.sumGOC()));
}
*/

//____________________________________________________________________________//

// EOF
