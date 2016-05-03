#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <ctime>
#include <assert.h>
#include <boost/program_options.hpp>

#include "Alignment.h"
#include "argParsing.h"
#include "nsga-ii.h"
#include "localSearch.h"
using namespace std;
namespace po = boost::program_options;



int main(int ac, char* av[])
{
	try{
		argRetVals vals = handleArgs(ac,av);
		po::variables_map vm = get<0>(vals);
		Network* net1 = get<1>(vals);
		Network* net2 = get<2>(vals);
		BLASTDict* bitPtr = get<3>(vals);
		BLASTDict* evalues = nullptr;
		GOCDict* gocPtr = get<5>(vals);
		vector<fitnessName> fitnessNames = get<6>(vals);

		const int nthreads = vm.count("nthreads") ? vm["nthreads"].as<int>()
		                                          : 1;
		const float mutswappb = vm.count("mutswappb")  
		                             ? vm["mutswappb"].as<float>()
		                             : 0.005;
		const float cxswappb = vm.count("cxswappb") ? vm["cxswappb"].as<float>()
		                                            : 0.1;
		const bool verbose = vm.count("verbose");
		const bool tournsel = vm.count("tournsel");
		const bool total = vm.count("total");
		const bool uniformsize = vm.count("uniformsize");
		const bool smallstart = vm.count("smallstart");
		const bool finalstats = vm.count("finalstats");
		const bool nooutput = vm.count("nooutput");
		const string outprefix = nooutput ? "" : vm["outprefix"].as<string>();

		const int generations = vm["generations"].as<int>();
		const int randseed = vm.count("randseed") ? vm["randseed"].as<int>() : clock();

		mt19937 g(randseed);
		//initialize population
		
		
		Alignment* aln = new Alignment(net1,net2, bitPtr, gocPtr);
		//aln->greedyMatch(false);
		aln->shuf(g,false,false,total);
		aln->computeFitness(fitnessNames);
		//steepestAscentHillClimb(aln, fitnessNames, nthreads, verbose);
		
		//todo: instead of just flipping obj, switch according to some
		//input time proportion.
		//fast hill climb version
		cout<<"starting main loop"<<endl;
		for(int i = 0; i < generations; i++){
			correctHillClimb(g, aln, total,
               500, fitnessNames);	
			
			for(int j = 0; j <fitnessNames.size();j++){
				cout<<"current "<<fitnessNameToStr(fitnessNames.at(j))<<" is "
				    <<aln->fitness.at(j)<<endl;
			}
            cout<<"EC is "<<((double)(aln->currConservedCount))/((double)(net1->edges.size()))<<endl;
			cout<<"Generation "<<i<<" complete."<<endl;
		}

		if(!nooutput)
			aln->save(outprefix + "_localTest.aln");
		
		
	}
	catch(exception& e){
		cerr << "error: " << e.what() << endl;
		cerr <<"Run with --help for help."<<endl;
	}



	return 0;
}
