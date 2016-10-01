#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <iomanip>
#include <sstream>

#include "argumentDescriptions.hpp"
using namespace std;

vector<array<string, 5>> argumentDescriptions = {
	{ "Option", "Type", "Default", "Title", "Description" },
	//-----------------------------------GENERAL-----------------------------------------
	{ "", "", "banner", "", "---------------------------------------- General Options ------------------------------------------" },
	{ "-g1", "string", "yeast", "Network 1", "First nerwork (smaller one)." },
	{ "-g2", "string", "human", "Network 2", "Second (larger in number of nodes) network." },
	{ "-fg1", "string", "yeast", "Network 1", "Initializes the network G1 with an external file. Make sure that the name and path of the file exists." },
	{ "-fg2", "string", "human", "Network 2", "Initializes the network G2 with an external file. Make sure that the name and path of the file exists. The network of G2 should have more nodes than G1." },
	{ "-o", "string", "sana", "Output File basename", "Specifies the basename of output file; actual output files will append (.out, .align, etc) to this." },
	{ "-t", "double", "5", "Runtime in minutes", "The number of minutes to run SANA. Must be non-zero, no upper limit." },
	{ "-i", "double", "0", "Iteration", "Like -t, but specifying total iteration count, in units of 100 million (Mutually exclusive with -t)." },
	{ "-pathmap1", "integer", "", "Path Map G1", "Allows mapping a path in G1 to an edge in G2, as if the path were a single edge in G2. Implemented by raising the adjacency list to this power (an integer)." },
	{ "-pathmap2", "string", "", "Path Map G2", "Maps a path in G2 to an edge in G1, as if the path were a single edge in G1." },
	{ "-eval", "string", "", "Evaluate Existing Alignment", "Takes an existing alignment, evaluates it, and records the results to sana.out or the specified output file." },
	{ "-startalignment", "string", "", "Starting Alignment", "File containing the starting alignment (in the format outputted by SANA). Some methods allow this option, while the rest start with random alignments." },
	{ "-truealignment", "string", "", "True Alignment", "Alignment file containing the \"true\" alignment. This is used to evaluate the NC measure. In its absence, NC assumes that the true alignment is the identity (the node with index i in G1 is mapped to the node with index i in G2). In any case, NC is expressed as the fraction of nodes in the smaller network aligned correctly." },
	{ "-rewire", "double", "0", "Add rewiring noise to G2", "If set greater than 0, the corresponding fraction of edges in G2 is randomly rewired." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//----------------------------------METHOD-------------------------------------------
	{ "", "", "banner", "", "\n-------------------------------------------- Method -----------------------------------------------" },
	{ "-method", "string", "sana", "Method", "Sets the algorithm that performs the alignment. All methods except \"sana\" call an external algorithm written by other authors. \nPossible aligners are: \"lgraal\", \"hubalign\", \"sana\", \"wave\", \"random\", \"tabu\", \"dijkstra\", \"netal\", \"mi-graal\", \"ghost\", \"piswap\", \"optnetalign\", \"spinal\", \"great\", \"netalie\", \"gedevo\", \"greedylccs\", \"magna\", \"waveSim\", \"none\", and \"hc\"." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//------------------------------------SANA-------------------------------------------
	{ "", "", "banner", "", "\n--------------- Additional options to consider when method is \"sana\" (the default) ----------------" },
	{ "-tinitial", "double", "1", "Initial Temperature", "Starting temperature of the simulated annealing. \"auto\" means calculate optimal starting temperature (requires extra CPU time)." },
	{ "-tdecay", "double", "1", "Rate of Decay", "Exponential decay parameter of the temperature schedule. \"auto\" means calculate optimal starting temperature (requires extra CPU time)." },
	{ "-combinedScoreAs", "string", "sum", "Score Combo Method", "If multiple objectives are specified, this specifies how to combine them. Choices are: sum, product, inverse, max, min, maxFactor." },
	{ "-dynamictdecay", "bool", "0", "Dynamically control temperature decay", "Whether or not tdecay is set to auto, this Boolean specifies if we should dynamically adjust the temperature schedule as the anneal progresses. Gives potentially better results than fixed decay rate." },
	{ "-lock", "string", "", "Node-to-Node Locking", "Specify a two column file of node pairs that are locked in the alignment." },
	{ "-seed", "double", "RANDOM", "Random Seed", "Serves as a random seed in SANA." },
	{ "-restart", "bool", "false", "(DEPRECATED)Restart Scheme", "(DEPRECATED)Active the restart scheme in SANA." },
	{ "-tnew", "double", "3", "(DEPRECATED)Restart Scheme t1", "(DEPRECATED)Parameter t1 of the restart scheme, in minutes" },
	{ "-iterperstep", "double", "10000000", "(DEPRECATED)Restart Scheme N", "(DEPRECATED)Parameter N of the restart scheme." },
	{ "-numcand", "double", "3", "(DEPRECATED)Restart Scheme K", "(DEPRECATED)Parameter K of the restart scheme." },
	{ "-tcand", "double", "1", "Restart Scheme t2(DEPRECATED)", "(DEPRECATED)Parameter t2 of the restart scheme, in minutes" },
	{ "-tfin", "double", "3", "Restart Scheme t3(DEPRECATED)", "(DEPRECATED)Parameter t3 of the restart scheme, in minutes" },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//------------------------------------TABU-------------------------------------------
	{ "", "", "banner", "", "\n---------------------- Additional options to consider when method is \"tabu\" -----------------------" },
	{ "-ntabus", "double", "300", "(DEPRECATED)TABU Argument", "(DEPRECATED)Argument for the TABU search function." },
	{ "-nneighbors", "double", "50", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU." },
	{ "-nodetabus", "bool", "false", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//---------------------------------Dijkstra------------------------------------------
	{ "", "", "banner", "", "\n-------------------- Additional options to consider when method is \"dijkstra\" ---------------------" },
	{ "-dijkstradelta","double","0","Dijkstra Randomness Amount","When picking the next edge to cross when adding a new node in the seed-and-extend Dijkstra algorithm, -dijkstradelta will set the width of the range of node similarity values that should be considered equal. Larger values inject more randomness; zero means \"pick best node according to current measure\"." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//-----------------------------------LGRAAL------------------------------------------
	{ "", "", "banner", "", "\n--------------------- Additional options to consider when method is \"lgraal\" ----------------------" },
	{ "-lgraaliter", "double", "1000", "LGRAAL Iteration", "Number of LGRAAL Iterations" },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------
	
	//----------------------------Objective Funtion Type---------------------------------
	{ "", "", "banner", "", "\n-------------------------------------- Objective Function -----------------------------------------" },
	{ "-objfuntype", "string", "generic", "Objective Function Type", "Acceptable arguments are \"alpha\", \"beta\", or \"generic\". Make sure the chosen argument of \"-objectfuntype\" also matches with the chosen argument for \"-method\"." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//---------------------Objective Funtion Type Specifications-------------------------
	{ "", "", "banner", "", "\n----------- Objective Function Specification and Weight. Combine with \"-objfuntype x\" -------------" },
	{ "-alpha", "double", "0", "alpha=0 means toplogy only; alpha=1 means sequence only.", "Trade off between biological (alpha = 1) and topological (alpha = 0) measures. Range: [0, 1]. Used when \"-objfuntype\" is \"alpha\"." },
	{ "-beta", "double", "0", "Like -alpha, but values are normalized first.", "Same as alpha but with topological and biological scores balanced according to size. Range: [0, 1]. Used when \"-objfuntype\" is \"beta\"." },
	{ "-ec", "double", "0", "Weight of EC", "The weight of the Edge Coverage (aka Edge Conservation or Edge Correctness) in the objective function. To be used when \"-objfuntype\" is \"generic\"." },
	{ "-s3", "double", "1", "Weight of S3", "The weight of the Symmetric Substructer Score in the objective function. To be used when \"-objfuntype\" is \"generic\"." },
	{ "-wec", "double", "0", "Weight of WEC.", "Weight of the weighted edge coverage. To be used when \"-objfuntype\" is \"generic\". If non-zero, must specify how to weigh the edge using -wecnodesim." },
	{ "-spc", "double", "0", "Shortest Path Conservation", "Objective function to minimize shortest path difference between pairs of nodes in G1 and G2. Used when \"-objfuntype\" is \"generic\"." },
	{ "-importance", "double", "0", "Weight of Importance", "HubAlign's objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-nodec", "double", "0", "Weight of Node Count", "The weight of the Local Node Count objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-noded", "double", "0", "Weight of Node Density", "The weight of the Local Node Density objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-edgec", "double", "0", "Weight of Edge Count", "The weight of the Local Edge Count objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-edged", "double", "0", "Weight of Edge Density", "The weight of the Local Edge Density objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-esim", "double", "0", "External Similarity Weight", "The weight of the external similarity file. Used when \"-objfuntype\" is \"generic\". (Pending changes to SANA, these values may be normalized to be in [0,1] after they're read but before they're used.)" },
	{ "-graphlet", "double", "0", "Weight of Graphlet Similarity.", "The weight of the Graphlet Objective Function as defined in the original GRAAL paper (2010). Used when \"-objfuntype\" is \"generic\"." },
	{ "-graphletlgraal", "double", "0", "Weight of Graphlet Similarity (LGRAAL)", "The weight of LGRAAL's objective function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-sequence", "double", "0", "Weight of Sequence Similarity", "The weight of the Sequence Similarity function. Used when \"-objfuntype\" is \"generic\"." },
	{ "-go_k", "double", "0", "k-common GO terms", "Objective function based on having up to k GO terms in common. Used when \"-objfuntype\" is \"generic\"." },
	{ "-graphletcosine", "double", "0", "Similarity of cosine between GDV vectors", "Objective function based on the cosine angle between graphlet degree vectors. Used when \"-objfuntype\" is \"generic\"." },
	{ "-topomeasure", "string", "", "Topological Measure", "Topological component of the scoring function. Used when \"-objfuntype\" is either \"alpha\" or \"beta\"." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//-----------------Further details of \"-method x -objfuntype y\"--------------------
	{ "", "", "banner", "", "\n-------------- Further Weight Specification. Combine with \"-method x -objfuntype y\" ---------------" },
	{ "-wrappedArgs", "string", "", "Wrapper Function Arguments", "Arguments to pass verbatim to wrapped methods." },
	{ "-maxDist", "double", "1", "Radial Distance from Node", "When using nodec, edgec, noded, or edged, the radial distance region over which to compute the count/density. Used when \"-objfuntype\" is \"generic\"." },
	{ "-gofrac", "double", "1", "Fraction of GO_k Terms to Keep", "Used for GO similarity (\"-go_k\"). It is the fraction of GO term ocurrences corresponding to the least frequent terms to be kept." },
	{ "-nodecweights", "vector", "4 .1 .25 .5 .15", "Weights of Node Density Measure", "Weights w of the Node count/density measure. They are automatically scaled to 1." },
	{ "-edgecweights", "vector", "4 .1 .25 .5 .15", "Weights of Edge Density Measure", "Weights w of the Edge count/density measure. They are automatically scaled to 1." },
	{ "-goweights", "vector", "1 1", "Weight and Measure of GO Measures", "Specifies the maximum GOk measure and the weight of each one." },
	{ "-wecnodesim", "string", "graphletlgraal", "Weighted Edge Coverage Node Pair Similarity", "Node pair similarity used to weight the edges in the WEC measure. The edges are weighted by taking the average of the scores of an edge's two ending nodes using some node similarity measure which can be different from the default node sim measure." },
	{ "-wavenodesim", "string", "nodec", "Weighted Average Node Pair Similarity", "Node pair similarity to use when emulating WAVE." },
	{ "-simFile", "string", "", "Similarity File", "Specify an external three columnn (node from G1, node from  G2, similarity) file. These will be given weight according to the -esim argument." },
	{ "-detailedreport", "bool", "false", "Detailed Report", "If false, initialize only basic measures and any measure necessary to run SANA." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//------------------------------------MODE-------------------------------------------
	{ "", "", "banner", "", "\n--------------------------------------------- Mode ------------------------------------------------" },
	{ "-mode", "string", "normal", "Mode", "Runs SANA in a specified mode. Arguments for this option are: \"cluster\", \"exp\", \"param\", \"alpha\", \"dbg\", \"normal\", \"analysis\", \"similarity\"." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//----------------------------------CLUSTER------------------------------------------
	{ "", "", "banner", "", "\n-------------------------------- More options for \"-mode cluster\" ---------------------------------" },
	{ "-qmode", "string", "normal", "Queue Mode", "The \"Queue Mode\" when using SGE" },
	{ "-qsuboutfile", "string", "", "Queue Mode Output File", "Output file when using SGE" },
	{ "-qsuberrfile", "string", "", "Queue Error File", "Error file when using SGE" },
	{ "-qsubscriptfile", "string", "", "Queue Script File", "Script file when using SGE" },
	{ "-qcount", "double", "1", "Qsub Count", "When submitting to SGE, number of times to submit the same job." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//--------------------------------EXPERIMENTAL---------------------------------------
	{ "", "", "banner", "", "\n---------------------------------- More options for \"-mode exp\" -----------------------------------" },
	{ "-outfolder", "string", "", "Experimental Mode Output Folder", "The folder SANA will output results to during Experiment Mode." },
	{ "-experiment", "string", "", "Experimental Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on experiment mode." },
	{ "-collect", "bool", "false", "SGE Results Collection", "Collects results from a running or finished SGE job." },
	{ "-dbg", "bool", "false", "Debug Flag", "Each mode in SANA might have a different use for the dbg flag (e.g., in experiments, if -dbg is found, the runs are not actually sent to the cluster, only printed)." },
	{ "-local", "bool", "false", "Submit Jobs Locally", "Used with SGE. Will not submit jobs to SGE, but jobs will be run locally instead." },
	{ "-updatecsv", "bool", "false", "Update CSV without re-running", "Used with expermient mode. If the data has already been collected, instead of re-evaluating all the alignments, it loads up the data and updates the csv output file." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//----------------------------PARAMETER ESTIMATION-----------------------------------
	{ "", "", "banner", "", "\n--------------------------------- More options for \"-mode param\" ----------------------------------" },
	{ "-paramestimation", "string", "", "Parameter Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode" },
	{ "-submit", "bool", "false", "Print Jobs Instead of Submitting", "Used with Debugging Mode for SGE. If false, SGE jobs will not be submitted to queues, but will be printed to console instead." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//------------------------------ALPHA ESTIMATION-------------------------------------
	{ "", "", "banner", "", "\n--------------------------------- More options for \"-mode alpha\" ----------------------------------" },
	{ "-alphaestimation", "string", "", "Alpha Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode" },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//-----------------------------------DEBUG-------------------------------------------
	//------------------------------------END--------------------------------------------

	//----------------------------------NORMAL-------------------------------------------
	//------------------------------------END--------------------------------------------

	//---------------------------------ANALYSIS------------------------------------------
	{ "", "", "banner", "", "\n------------------------------- More options for \"-mode analysis\" ---------------------------------" },
	{ "-alignFormat", "integer", "0", "Alignfile Format", "Used in Analysis Mode \"-mode analysis\"." },
	{ "-alignFile", "string", "", "Alignment File Analysis Mode", "Used with \"-mode analysis\" to specify which pre-existing alignment file is being analyzed. Allowed values are 1=sana.out; 2=edge list; 3=partial edge list; 4=mapping (one line)." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------

	//---------------------------------SIMILARITY----------------------------------------
	{ "", "", "banner", "", "\n------------------------------ More options for \"-mode similarity\" --------------------------------" },
	{ "-simFormat", "double", "1", "Similarity File Format", "Used in Similarity Mode \"-mode similarity\" and with \"-objfuntype -esim\". Allowed values are 1=node names; 2=node integers numbered as in LEDA .gw format." },
	{ "---------------------------------------------- End ------------------------------------------------\n", "", "banner", "", "" },
	//------------------------------------END--------------------------------------------
};

void printAllArgumentDescriptions()
{	
	ifstream helpOutput;
	helpOutput.open("./src/arguments/helpOutput");
	string line;
	getline(helpOutput, line);
	while (line != "OPTIONS, ARGS, AND DESCRIPTIONS GO HERE.")
	{
		cerr << line << endl;
		getline(helpOutput, line);
	}
	
	for (auto it = argumentDescriptions.begin(); it != argumentDescriptions.end(); it++)
	{
		cerr << printItem(*it);
	}
	
	getline(helpOutput, line);
	cerr << line;
	while (!helpOutput.eof())
	{
		getline(helpOutput, line);
		cerr << endl << line;
	}
	
	helpOutput.close();
}

string printItem(const array<string, 5> &item)
{
	ostringstream toReturn;
	toReturn << '\n';
	if (item[2] == "banner")
		toReturn << item[0] << item[4];
	else
	{
		toReturn << ' ' << setw(17) << left << item[0] << setw(8) << left << item[1] << setw(16) << left;
		if (item[2].length() == 0)
			toReturn << "\"\"";
		else
			toReturn << item[2];
		toReturn << item[3] << '\n' << formatDescription(item[4]);
	}
	return toReturn.str();
}

string formatDescription(string description)
{
	ostringstream toReturn;
	int end = 57;
	while (description.length() > 57)
	{
		while (description[end] != ' ')
		{
			end--;
		}
		toReturn << setw(42) << left << "" << description.substr(0, end) + '\n';
		description = description.substr(end + 1, description.length());
		end = 57;
	}
	toReturn << setw(42) << left << "" << description;
	return toReturn.str();
}
