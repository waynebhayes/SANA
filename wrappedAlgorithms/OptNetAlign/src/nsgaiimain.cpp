#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <array>
#include <algorithm>
#include <unordered_set>
#include <ctime>
#include <boost/thread/thread.hpp>
#include <assert.h>
#include <boost/program_options.hpp>

#include "Alignment.h"
#include "argParsing.h"
#include "nsga-ii.h"
#include "localSearch.h"

#include "tbb/tbb.h"
using namespace std;
using namespace tbb;
namespace po = boost::program_options;



int main(int ac, char* av[])
{
	try{
		argRetVals vals = handleArgs(ac,av);
		po::variables_map vm = get<0>(vals);
		Network* net1 = get<1>(vals);
		Network* net2 = get<2>(vals);
		const BLASTDict* bitPtr = get<3>(vals);
        const GOCDict* gocsPtr = get<5>(vals);
		vector<fitnessName> fitnessNames = get<6>(vals);

		
		const float mutswappb = vm.count("mutswappb")  
		                             ? vm["mutswappb"].as<float>()
		                             : 0.005;
		const float cxswappb = vm.count("cxswappb") ? vm["cxswappb"].as<float>()
		                                            : 0.1;
		const float cxrate = vm.count("cxrate") ? vm["cxrate"].as<float>() : 0.7;
		const bool verbose = vm.count("verbose");
		const bool tournsel = vm.count("tournsel");
		const bool total = vm.count("total");
		const bool uniformsize = vm.count("uniformsize");
		const bool smallstart = vm.count("smallstart");
		const bool finalstats = vm.count("finalstats");
		const bool seeding = vm.count("seeding");
        const bool nooutput = vm.count("nooutput");
		const string outprefix = vm.count("outprefix") ? 
                                 vm["outprefix"].as<string>()
                                 : "nooutprefixgiven";
		const int hillclimbiters = vm.count("hillclimbiters") 
		                           ? vm["hillclimbiters"].as<int>() 
		                           : 0; 

		RandGenT g(14);
		//initialize population
		if(verbose){
			cout<<"creating initial population"<<endl;
		}
		const unsigned int popsize = vm.count("popsize") 
		                             ? vm["popsize"].as<int>()
		                             : 100;
		vector<Alignment*> pop(popsize,nullptr);
		vector<Alignment*> kids(popsize,nullptr);

        //hillclimb initialization is slow, so we will make it multithreaded

        //set up a list of proportions to use for each aln
        vector<double> proportions(popsize);
        for(int i = 0; i < proportions.size(); i++){
            proportions[i] = double(i)/double(popsize);
        }

        auto worker = [&](const blocked_range<size_t>& r){
            RandGenT tg(clock());
            if(seeding){
                int numSearchIters = 1000000;
                for(int i = r.begin(); i != r.end(); ++i){
                    pop[i] = new Alignment(net1,net2,bitPtr,gocsPtr);
                    pop[i]->shuf(tg,false,false,total);
                    pop[i]->computeFitness(fitnessNames);
                    if(fitnessNames.size()>1){
                        proportionalSearch(tg, pop[i], total,
                            numSearchIters, fitnessNames,
                            0, proportions[i]);
                    }
                    else{
                        correctHillClimb(tg, pop[i], total,
                        numSearchIters, fitnessNames);
                    }
                    kids[i] = new Alignment(net1,net2,bitPtr,gocsPtr);
                    kids[i]->shuf(tg,false,false,total);
                    kids[i]->computeFitness(fitnessNames);
                    if(fitnessNames.size()>1){
                        proportionalSearch(tg, kids[i], total,
                            numSearchIters, fitnessNames,
                            0, proportions[i]);
                    }
                    else{
                        correctHillClimb(tg, kids[i], total,
                        numSearchIters, fitnessNames);
                    }
                }
            }
            else{
                for(int i = r.begin(); i != r.end(); ++i){
                    pop[i] = new Alignment(net1,net2, bitPtr,gocsPtr);
                    pop[i]->shuf(tg, uniformsize, smallstart, total);
                    pop[i]->computeFitness(fitnessNames);
                    kids[i] = new Alignment(net1,net2, bitPtr,gocsPtr);
                    kids[i]->shuf(tg, uniformsize, smallstart, total);
                    kids[i]->computeFitness(fitnessNames);
                }
            }

        };

        parallel_for(blocked_range<size_t>(0,popsize),worker);

        //one last thing: create a greedy matching.
        //prefers GOC if it is defined
        if(gocsPtr && seeding){
        	pop[popsize-1] = new Alignment(net1,net2,bitPtr,gocsPtr);
        	pop[popsize-1]->greedyMatch(false);
        	pop[popsize-1]->computeFitness(fitnessNames);
        }
        else if(bitPtr && seeding){
            pop[popsize - 1] = new Alignment(net1,net2,bitPtr,gocsPtr);
            pop[popsize - 1]->greedyMatch(true);
            pop[popsize - 1]->computeFitness(fitnessNames);
        }
		

		//main loop
		if(verbose){
			cout<<"starting main loop"<<endl;
		}
		const int generations = vm["generations"].as<int>();
		for(int gen = 0; gen < generations; gen++){

			//combinedPtrs is R_t from Deb et al. 2002

			//todo: do this as a more idiomatic concatenation
			vector<Alignment*> combinedPtrs(2*popsize);
			for(int i = 0; i < popsize; i++){
				combinedPtrs[i] = pop[i];
			}
			for(int i = popsize; i < 2*popsize; i++){
				Alignment* ptr = kids.at(i-popsize);
				combinedPtrs.at(i) = ptr;
			}
			

			vector<vector<Alignment*> > fronts = nonDominatedSort(combinedPtrs);

			normalizeFitnesses(combinedPtrs);
			//started with best front, add to new population front-by-front
			unordered_set<Alignment*> popNew;
			popNew.reserve(popsize);
			int i = 0;
			while(popNew.size() + fronts[i].size() < popsize){
				setCrowdingDists(fronts[i]);
				popNew.insert(fronts[i].begin(), fronts[i].end());
				i++;
			}

			//add the least-crowded members of the front that doesn't
			//completely fit given our popsize.
			int numLeftToInsert = popsize - popNew.size();
			if(numLeftToInsert > 0){
				setCrowdingDists(fronts[i]);
				sort(fronts[i].begin(),fronts[i].end(),crowdedComp);
				popNew.insert(fronts[i].begin(), 
				              fronts[i].begin() + numLeftToInsert);
			}

			//go through combinedPtrs, deleting those not in popNew
			for(int i = 0; i < combinedPtrs.size(); i++){
				if(!popNew.count(combinedPtrs[i])){
					delete combinedPtrs[i];
				}
			}
			//set pop = popNew
			copy(popNew.begin(), popNew.end(), pop.begin());

			//do multithreaded version of kids creation
			//first resize kids to the proper size
			kids = vector<Alignment*>(popsize);

			//this will be executed by each thread.
			auto worker = [&](const blocked_range<size_t>& r){
				RandGenT tg(clock());
				for(auto i = r.begin(); i != r.end(); ++i){
					uniform_real_distribution<double> dist(0.0,1.0);
					double prob = dist(tg);
					if(prob <= cxrate){
						vector<Alignment*> parents;
						if((popsize/10) > 2 && tournsel){
							parents = binSel(tg,pop,(popsize/10));
						}
						else{
							uniform_int_distribution<int> rint(0,popsize-1);
							int par1 = rint(tg);
							int par2 = par1;
							while(par2 == par1){
								par2 = rint(tg);
							}
							parents.push_back(pop[par1]);
							parents.push_back(pop[par2]);
						}
						kids[i] = new Alignment(tg,cxswappb,*parents[0],
							               *parents[1], total);
						if(prob > 0.2){
							kids[i]->mutate(tg,mutswappb,total);
						}
					}
					else{
						vector<Alignment*> parents = binSel(tg,pop,(popsize/10));
						kids[i] = new Alignment(*parents[0]);
						kids[i]->mutate(tg,mutswappb,total);
					}
					kids[i]->computeFitness(fitnessNames);

					//do local search by hill-climbing 
					if(hillclimbiters != 0){
						  //this hillclimb tries to improve all objectives. May want to try
						  //a version that optimizes one at expense of others, or something
						  correctHillClimb(tg, kids[i], total,
	                         hillclimbiters, fitnessNames);
					}
				}

			};

			parallel_for(blocked_range<size_t>(0,popsize),worker);

			
			if(verbose){
				cout<<"Finished generation "<<gen<<endl;
				reportStats(pop,fitnessNames,verbose,false);
				cout<<endl;
			}
		}

		if(verbose && !nooutput){
			cout<<"Finished!"<<endl;
			cout<<"Writing alignments in Pareto front"<<endl;
		}
		if(finalstats){
			cout<<popsize;
			cout<<'\t'<<generations;
			cout<<'\t'<<mutswappb;
			cout<<'\t'<<cxswappb;
			cout<<'\t'<<tournsel;
			cout<<'\t'<<uniformsize;
			reportStats(pop,fitnessNames,verbose,false);
			cout<<endl;
		}
		vector <Alignment*> allAlns;
		allAlns.reserve(popsize*2);
		allAlns.insert(allAlns.end(), pop.begin(), pop.end());
		allAlns.insert(allAlns.end(), kids.begin(), kids.end());
		vector<vector<Alignment* > > fronts = nonDominatedSort(allAlns);
        
        if(!nooutput){
            string infoFilename = outprefix + ".info";
            ofstream infoFile(infoFilename);

            //make infoFile column labels
            infoFile << "filename";

            for(auto str : fitnessNames){
                infoFile << '\t' << str;
            }

            infoFile << endl;

            //output all alignments in the first front
            for(int i = 0; i < fronts[0].size(); i++){
                string filename = outprefix + "_" + to_string(i) + ".aln";
                fronts[0][i]->save(filename);

                infoFile << filename;

                //write summary info to infoFile
                for(int j = 0; j < fronts[0][i]->fitness.size(); j++){
                    infoFile << '\t' << fronts[0][i]->fitness[j];
                }

                infoFile << endl;
            }
        }

	}
	catch(exception& e){
		cerr << "error: " << e.what() << endl;
		cerr <<"Run with --help for help."<<endl;
	}

	return 0;
}
