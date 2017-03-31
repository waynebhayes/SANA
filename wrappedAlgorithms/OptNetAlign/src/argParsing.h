#pragma once

#include <exception>
#include <string>
#include <tuple>
#include <boost/program_options.hpp>

#include "goc.h"
#include "Network.h"
#include "blastinfo.h"
using namespace std;
namespace po = boost::program_options;

class ArgError : public exception{
public:

	ArgError(string msg){
		errMsg = msg;
	}

	virtual const char* what() const throw(){
		return errMsg.c_str();
	}

private:
	string errMsg;
};

typedef tuple<po::variables_map, Network*, Network*,
              BLASTDict*, BLASTDict*, GOCDict*, 
              vector<fitnessName> > argRetVals; 


argRetVals handleArgs(int ac, char* av[]){
	Network *net1, *net2;
	po::options_description desc("Options");
	//todo: add to help descriptions to indicate which args optional.
	desc.add_options()
		("help", "Displays this help message.")
		("net1", po::value<string>(), "Path to first network file. "
			                          "One interaction per line,"
			                          "represented as two protein names separated "
			                          "by whitespace. See documentation for an "
			                          "example. The network may at most have as "
			                          "many nodes as net2.")
		("net2", po::value<string>(), "Path to the second network file. "
			                          "Same format expected as first network "
			                          "file.")
		("outprefix", po::value<string>(), "Prefix for all output files.")
		("total", "When set, restricts alignments to total alignment only."
			      " Otherwise, one of the objectives will be alignment size")
		("uniformsize", "When set, and --total is not, the initial population"
			            " will consist of alignments generated so that their"
			            " sizes are drawn from a uniform distribution."
			            " Otherwise, each will have about half of their "
			            "nodes aligned.")
		("smallstart", "When set, and --total and --uniformsize are not, "
			           "initialize the population with random alignments " 
			           "that are very small compared to the size of "
			           "net1. These will be up to 1/100th the size of net1.")
		("popsize", po::value<int>(), "The number of alignments to maintain "
			                          "in the genetic algorithm's population."
			                          " Default: 100")
		("generations", po::value<int>(), "The max number of generations to "
			                              "run the algorithm.")
		("initlist", po::value<string>(), "Path to a list of paths to existing "
			                              "alignments to use to seed the search. "
			                              "Optional.")
		("bitscores", po::value<string>(), "Path to a list of BLAST bit scores "
			                               "for the given networks. Each line "
			                               "should be the names of two nodes and "
			                               "their bitscore, whitespace-separated."
			                               "When this argument is used, the bit "
			                               "score information will be used in "
			                               "creating alignments.")
		("evalues", po::value<string>(), "Path to E-values for the given "
			                             "networks. Uses the same format as "
			                             "bit scores data.")
		("annotations1", po::value<string>(), "Path to a list of GO annotations "
			                                  "for network 1. Each line should "
			                                  "be a node name followed by its "
			                                  "GO annotations, separated by "
			                                  "whitespace. When a path to "
			                                  "annotations is specified for both "
			                                  "networks, this info is used as "
			                                  "an objective to guide the "
			                                  "alignment.")
		("annotations2", po::value<string>(), "Path to GO annotations for net 2.")
		("goc", "When set, GOC will be used as an aligment objective. If set, "
			    "annotations1 and annotations2 must also be provided.")
		("blastsum", "When set, the sum of bitscores or E-values of aligned pairs "
			            "will be used as an alignment objective. If set, "
			            "the path to the bitscores must be provided with "
			            "the --bitscores switch, or the path to E-values "
			            "must be provided with the --evalues switch.")
		("ics", "When set, integrated conserved structure score will be used as "
			    "an alignment objective.")
		("ec", "When set, edge correctness score will be used as an "
			   "alignment objective.")
        ("s3", "When set, symmetric substructure score will be used as"
               " an alignment objective.")
        ("s3denom", "When set, s3's denominator will be used as an "
        	        "alignment objective to be minimized.")
		("verbose", "When set, extra information about the progress of the "
			        "alignment is printed to stdout.")
		("mutswappb", po::value<float>(), "Sets the probability of perturbing an "
			                              "aligned pair when performing mutation. "
			                              "See documentation."
			                              " Default: 0.005")
		("mutrate", po::value<float>(), "Sets the probability mutation will be "
										"performed when creating a new alignment. "
										"Default: 0.1.")
		("cxswappb", po::value<float>(), "Sets the probability of swapping "
			                             "during crossover. See documentation."
			                             " Default: 0.1")
		("cxrate", po::value<float>(), "Sets the probability that crossover "
			                           "will be performed when creating a new "
			                           "alignment. Default: 0.7.")
		("oneobjrate", po::value<float>(), "Sets the probability that one "
										   "objective hillclimbing will be "
										   "used instead of non-dominated "
										   "hillclimbing. Default: 0.1")
		("finalstats", "When set, prints comma-separated stats to stdout "
			           "when execution completes. Used to find good parameter "
			           "values through experimentation.")
		("seeding", "When set, attempts to start with better-than-random "
			        "alignments by creating randomized local search "
			        "alignments to serve as the initial population. ")
        ("nooutput", "When set, does not output final alignments to" 
                     " disk.")
		("tournsel", "When set, use tournament selection "
			         "to choose parents for crossover. Otherwise, uses "
			         "random selection.")
		("hillclimbiters", po::value<int>(), "Sets the number of hill-climbing"
			                                 " iterations to perform on each "
			                                 "member of each new generation."
			                                 "Default: 0.")
		("randseed", po::value<int>(), "Specifies random seed. Default: current time.")
        ("nthreads", po::value<int>(), "Sets number of threads to use. "
                                       "Default: number of cores on "
                                       "your machine.")
        ("dynparams", "When set, automatically adjust some parameters based on their "
        			  "success rate. See documentation for more information.")
        ("timelimit", po::value<int>(), "Sets a time limit in minutes. Default: not set.")
        ("icstimesec", "When set, uses the product of ICS and EC as an alignment objective.")
        ("s3variant", "When set, uses a variant on S^3 as an alignment objective.")
	;
	
	po::variables_map vm;
	po::store(po::parse_command_line(ac,av,desc), vm);
	po::notify(vm);

	if(vm.count("help")){
		cout<<desc<<endl;
		throw ArgError("Please run with valid arguments.");
	}


	if(!(vm.count("ics") || vm.count("bitscores") || vm.count("evalues")
		|| vm.count("ec") || vm.count("s3") || vm.count("s3denom")
		|| vm.count("icstimesec") || vm.count("s3variant"))
		&& !(vm.count("annotations1") && vm.count("annotations2")) ){
		throw ArgError("At least one objective must be specified!");
	}

	if((vm.count("annotations1") && !vm.count("annotations2")) ||
	   (vm.count("annotations2") && !vm.count("annotations1"))){
		throw ArgError("Please specify annotations for both networks "
			           "or neither.");
	}

	if(vm.count("seeding") && !(vm.count("bitscores") || vm.count("evalues") || vm.count("annotations2"))){
		throw ArgError("Seeding requires either bitscore or E-value data.");
	}


	if(!vm.count("net1") || !vm.count("net2")){
		throw ArgError("Both net1 and net2 must be specified.");
		//throw ArgError(vm["net1"].as<string>;
	}

	if(!vm.count("outprefix") && !vm.count("nooutput")){
		throw ArgError("outprefix must be specified.");
	}

	if(vm.count("bitscores") && vm.count("evalues")){
		throw ArgError("Currently only one of bitscores and E-values"
			           " can be optimized, not both.");
	}

	if(vm.count("blastsum") && !(vm.count("bitscores") || vm.count("evalues"))){
		throw ArgError("Cannot specify blastsum objective without providing a "
			           "path to either bitscores or E-values.");
	}

	if(vm.count("goc") && !(vm.count("annotations1") && vm.count("annotations2"))){
		throw ArgError("Cannot specify GOC objective without providing paths to "
			           "GO annotations using the --annotations1 and --annotations2 "
			           "parameters.");
	}

	net1 = new Network(vm["net1"].as<string>());
	net2 = new Network(vm["net2"].as<string>());
	int net1Size = net1->nodeToNodeName.size();
	int net2Size = net2->nodeToNodeName.size();
	
	if(net1->nodeToNodeName.size() > net2->nodeToNodeName.size()){
		throw ArgError("Number of nodes in net1 must be less than "
			           "or equal to the number of nodes in net2.");
	}
	
	vector<fitnessName> fitnessNames;

	if(vm.count("ics")){
		fitnessNames.push_back(ICSFit);
	}

	if(vm.count("ec")){
		fitnessNames.push_back(ECFit);
	}

    if(vm.count("s3")){
        fitnessNames.push_back(S3Fit);
    }

    if(vm.count("s3denom")){
    	fitnessNames.push_back(S3DenomFit);
    }

    if(vm.count("icstimesec")){
    	fitnessNames.push_back(ICSTimesEC);
    }

    if(vm.count("s3variant")){
    	fitnessNames.push_back(S3Variant);
    }

	BLASTDict* bitscores = nullptr;

	if(vm.count("bitscores")){
		bitscores = loadBLASTInfo(net1,net2,vm["bitscores"].as<string>());
		
	}

	if(vm.count("blastsum") && vm.count("bitscores")){
		fitnessNames.push_back(BitscoreSumFit);
	}

	BLASTDict* evalues = nullptr;

	if(vm.count("evalues")){
		evalues = loadBLASTInfo(net1,net2,vm["evalues"].as<string>());
	}

	if(vm.count("blastsum") && vm.count("evalues")){
		fitnessNames.push_back(EvalsSumFit);
	}
    
    GOCDict* gocs = nullptr;
    
    if(vm.count("annotations1") && vm.count("annotations2")){
        gocs = loadGOC(net1,net2,vm["annotations1"].as<string>(),
                           vm["annotations2"].as<string>());
        if(vm.count("goc")){
        	fitnessNames.push_back(GOCFit);
        }
    }

	if(!vm.count("total")){
		fitnessNames.push_back(SizeFit);
	}

	if(!vm.count("generations")){
		throw ArgError("Number of generations must be specified.");
	}

	if(vm.count("total") && 
	   (vm.count("smallsize") || vm.count("uniformsize"))){
		throw ArgError("--smallsize and --uniformsize not compatible with "
			           "--total.");
	}

	if(vm.count("smallsize") && vm.count("uniformsize")){
		throw ArgError("only one of --smallsize and --uniformsize "
			           "may be specified.");
	}

	return argRetVals(vm,net1,net2,bitscores,evalues,gocs,fitnessNames);
}
