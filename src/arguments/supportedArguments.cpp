#include "supportedArguments.hpp"

using namespace std;

vector<string> stringArgs;
vector<string> doubleArgs;
vector<string> boolArgs;
vector<string> vectorArgs;

//This file contains every argument supported by SANA contained basically inside an array, each element in the array contains 6 fields.
//A Description of each field:
//	"Option" is the name we use to set the value of a specific option when executing SANA. Example: ./sana -Option ARG
//	"Type" Describes what the datatype/datastructure is expected as an argument for that option.
//	"Default" Means what the value of the default argument for an option is (if unspecified when ./sana is invoked).
//	"Title" Gives a more general title of the option that serves to describe the option better than the first field "Option" does.
//	"Description" Provides a small description for the option.
//	"0/1", or the Last field is, for each element entry, either a '0' or '1'. '1' means the option can be accessed from other interfaces controlling SANA (when SANA is NOT run from a shell). A '0' means it is not accessible.

vector<array<string, 6>> supportedArguments = {
	{ "Option", "Type", "Default", "Title", "Description", "0/1" },
	//-----------------------------------GENERAL-----------------------------------------
	{ "", "", "banner", "", "General Options", "0" },
	{ "-g1", "string", "yeast", "Network 1", "First nerwork (smaller one). Requirement: An alignment file must exist inside the networks directory which matches the name of the specified species.", "0" },
	{ "-g2", "string", "human", "Network 2", "Second (larger in number of nodes) network.  Requirement: An alignment file must exist inside the networks directory which matches the name of the specified species.", "0" },
	{ "-fg1", "string", "yeast", "Network 1", "Initializes the network G1 with an external file. Make sure that the name and path of the file exists.", "1" },
	{ "-fg2", "string", "human", "Network 2", "Initializes the network G2 with an external file. Make sure that the name and path of the file exists. The network of G2 should have more nodes than G1.", "1" },
	{ "-o", "string", "sana", "Output File basename", "Specifies the basename of output file; actual output files will append (.out, .align, etc) to this.", "0" },
  { "-localScoresFile", "string", "sana", "Local Scores File basename", "Specifies the basename of the local score file; actual output files will append (.out, .align, etc) to this.", "0"},
	{ "-t", "double", "5", "Runtime in minutes", "The number of minutes to run SANA. Must be non-zero, no upper limit.", "1" },
	{ "-pathmap1", "integerS", "", "Path Map G1", "Allows mapping a path in G1 to an edge in G2, as if the path were a single edge in G2. Implemented by raising the adjacency list to this power (an integer).", "1" },
	{ "-pathmap2", "integerS", "", "Path Map G2", "Maps a path in G2 to an edge in G1, as if the path were a single edge in G1.", "1" },
	{ "-eval", "string", "", "Evaluate Existing Alignment", "Takes an existing alignment, evaluates it, and records the results to sana.out or the specified output file.", "1" },
	{ "-startalignment", "string", "", "Starting Alignment", "File containing the starting alignment (in the format outputted by SANA). Some methods allow this option, while the rest start with random alignments.", "1" },
	{ "-truealignment", "string", "", "True Alignment", "Alignment file containing the \"true\" alignment. This is used to evaluate the NC measure. In its absence, NC assumes that the true alignment is the identity (the node with index i in G1 is mapped to the node with index i in G2). In any case, NC is expressed as the fraction of nodes in the smaller network aligned correctly.", "0" },
	{ "-rewire", "double", "0", "Add rewiring noise to G2", "If set greater than 0, the corresponding fraction of edges in G2 is randomly rewired.", "0" },
	{ "End General Options", "", "banner", "", "", "0" },
	//---------------------------------END GENERAL---------------------------------------

	//------------------------------------METHOD-----------------------------------------
	{ "", "", "banner", "", "Method", "0" },
	{ "-method", "string", "sana", "Method", "Sets the algorithm that performs the alignment. NOTE: All methods except \"sana\" call an external algorithm written by other authors. \nPossible aligners are: \"lgraal\", \"hubalign\", \"sana\", \"wave\", \"random\", \"tabu\", \"dijkstra\", \"netal\", \"mi-graal\", \"ghost\", \"piswap\", \"optnetalign\", \"spinal\", \"great\", \"natalie\", \"gedevo\", \"greedylccs\", \"magna\", \"waveSim\", \"none\", and \"hc\".", "1" },
	{ "End Method", "", "banner", "", "", "0" },
	//----------------------------------END METHOD---------------------------------------

	//------------------------------------SANA-------------------------------------------
	{ "", "", "banner", "", "Additional options to consider when method is \"sana\" (the default)", "0" },
	{ "-tinitial", "doubleS", "auto", "Initial Temperature", "Starting temperature of the simulated annealing. \"auto\" means calculate optimal starting temperature (requires extra CPU time).", "1" },
	{ "-tdecay", "doubleS", "auto", "Rate of Decay", "Exponential decay parameter of the temperature schedule. \"auto\" means calculate optimal starting temperature (requires extra CPU time).", "1" },
	{ "-combinedScoreAs", "string", "sum", "Score Combo Method", "If multiple objectives are specified, this specifies how to combine them. Choices are: sum, product, inverse, max, min, maxFactor.", "1" },
	{ "-dynamictdecay", "bool", "0", "Dynamically control temperature decay", "Whether or not tdecay is set to auto, this Boolean specifies if we should dynamically adjust the temperature schedule as the anneal progresses. Gives potentially better results than fixed decay rate.", "1" },
	{ "-lock", "string", "", "Node-to-Node Locking", "Specify a two column file of node pairs that are locked in the alignment.", "0" },
	{ "-seed", "double", "RANDOM", "Random Seed", "Serves as a random seed in SANA.", "0" },
	{ "-restart", "bool", "false", "(DEPRECATED)Restart Scheme", "(DEPRECATED)Active the restart scheme in SANA.", "0" },
	{ "-tnew", "double", "3", "(DEPRECATED)Restart Scheme t1", "(DEPRECATED)Parameter t1 of the restart scheme, in minutes", "0" },
	{ "-iterperstep", "double", "10000000", "(DEPRECATED)Restart Scheme N", "(DEPRECATED)Parameter N of the restart scheme.", "0" },
	{ "-numcand", "double", "3", "(DEPRECATED)Restart Scheme K", "(DEPRECATED)Parameter K of the restart scheme.", "0" },
	{ "-tcand", "double", "1", "Restart Scheme t2(DEPRECATED)", "(DEPRECATED)Parameter t2 of the restart scheme, in minutes", "0" },
	{ "-tfin", "double", "3", "Restart Scheme t3(DEPRECATED)", "(DEPRECATED)Parameter t3 of the restart scheme, in minutes", "0" },
	{ "END Additional options to consider when method is \"sana\" (the default)", "", "banner", "", "", "0" },
	//----------------------------------END SANA-----------------------------------------

	//------------------------------------TABU-------------------------------------------
	{ "", "", "banner", "", "Additional options to consider when method is \"tabu\"", "0" },
	{ "-ntabus", "double", "300", "(DEPRECATED)TABU Argument", "(DEPRECATED)Argument for the TABU search function.", "0" },
	{ "-nneighbors", "double", "50", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU.", "0" },
	{ "-nodetabus", "bool", "false", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU.", "0" },
	{ "END Additional options to consider when method is \"tabu\"", "", "banner", "", "", "0" },
	//----------------------------------END TABU-----------------------------------------

	//----------------------------------Dijkstra-----------------------------------------
	{ "", "", "banner", "", "Additional options to consider when method is \"dijkstra\"", "0" },
	{ "-dijkstradelta","double","0","Dijkstra Randomness Amount","When picking the next edge to cross when adding a new node in the seed-and-extend Dijkstra algorithm, -dijkstradelta will set the width of the range of node similarity values that should be considered equal. Larger values inject more randomness; zero means \"pick best node according to current measure\".", "0" },
	{ "END Additional options to consider when method is \"dijkstra\"", "", "banner", "", "", "0" },
	//---------------------------------END Dikstra---------------------------------------

	//-----------------------------------LGRAAL------------------------------------------
	{ "", "", "banner", "", "Additional options to consider when method is \"lgraal\"", "0" },
	{ "-lgraaliter", "double", "1000", "LGRAAL Iteration", "Number of LGRAAL Iterations. This option is passed along to the external LGRAAL program.", "0" },
	{ "End Additional options to consider when method is \"lgraal\"", "", "banner", "", "", "0" },
	//---------------------------------END LGRAAL----------------------------------------

	//---------------------------Objective Function Type---------------------------------
	{ "", "", "banner", "", "Objective Function", "0" },
	{ "-objfuntype", "string", "generic", "Objective Function Type", "Acceptable arguments are \"alpha\", \"beta\", or \"generic\". Make sure the chosen argument of \"-objectfuntype\" also matches with the chosen argument for \"-method\".", "1" },
	{ "End Objective Function", "", "banner", "", "", "0" },
	//-------------------------END Objective Function Type-------------------------------

	//--------------------Objective Function Type Specifications-------------------------
	{ "", "", "banner", "", "Objective Function Specification and Weight. Combine with \"-objfuntype x\"", "0" },
	{ "-alpha", "double", "0", "alpha=0 means toplogy only; alpha=1 means sequence only.", "Trade off between biological (alpha = 1) and topological (alpha = 0) measures. Range: [0, 1]. Used when \"-objfuntype\" is \"alpha\".", "1" },
	{ "-beta", "double", "0", "Like -alpha, but values are normalized first.", "Same as alpha but with topological and biological scores balanced according to size. Range: [0, 1]. Used when \"-objfuntype\" is \"beta\".", "1" },
	{ "-ec", "double", "0", "Weight of EC", "The weight of the Edge Coverage (aka Edge Conservation or Edge Correctness) in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-s3", "double", "1", "Weight of S3", "The weight of the Symmetric Substructer Score in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-nc", "double", "0", "Weight of NC", "This weight of Node Correctness in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-wec", "double", "0", "Weight of WEC.", "Weight of the weighted edge coverage. To be used when \"-objfuntype\" is \"generic\". If non-zero, must specify how to weigh the edge using -wecnodesim.", "1" },
	{ "-spc", "double", "0", "Shortest Path Conservation", "Objective function to minimize shortest path difference between pairs of nodes in G1 and G2. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-importance", "double", "0", "Weight of Importance", "HubAlign's objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-nodec", "double", "0", "Weight of Node Count", "The weight of the Local Node Count objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-noded", "double", "0", "Weight of Node Density", "The weight of the Local Node Density objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-edgec", "double", "0", "Weight of Edge Count", "The weight of the Local Edge Count objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-edged", "double", "0", "Weight of Edge Density", "The weight of the Local Edge Density objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-esim", "double", "0", "External Similarity Weight", "The weight of the external similarity file. Used when \"-objfuntype\" is \"generic\". (Pending changes to SANA, these values may be normalized to be in [0,1] after they're read but before they're used.)", "1" },
	{ "-ewec", "double", "0", "External Weighted Edge Similarity", "The weighted of the external edge similarity file.", "1" },
        { "-graphlet", "double", "0", "Weight of Graphlet Similarity.", "The weight of the Graphlet Objective Function as defined in the original GRAAL paper (2010). Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-graphletlgraal", "double", "0", "Weight of Graphlet Similarity (LGRAAL)", "The weight of LGRAAL's objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-sequence", "double", "0", "Weight of Sequence Similarity", "The weight of the Sequence Similarity function. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-go_k", "double", "0", "k-common GO terms", "Objective function based on having up to k GO terms in common. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-graphletcosine", "double", "0", "Similarity of cosine between GDV vectors", "Objective function based on the cosine angle between graphlet degree vectors. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-topomeasure", "string", "", "Topological Measure", "Topological component of the scoring function. Used when \"-objfuntype\" is either \"alpha\" or \"beta\".", "1" },
	{ "End Objective Function Specification and Weight. Combine with \"-objfuntype x\"", "", "banner", "", "", "0" },
	//-------------------END Objective Function Tyoe Specifications----------------------

	//-----------------Further details of \"-method x -objfuntype y\"--------------------
	{ "", "", "banner", "", "Further Weight Specification. Combine with \"-method x -objfuntype y\"", "0" },
	{ "-wrappedArgs", "string", "", "Wrapper Function Arguments", "Arguments to pass verbatim to wrapped methods.", "0" },
	{ "-maxDist", "double", "1", "Radial Distance from Node", "When using nodec, edgec, noded, or edged, the radial distance region over which to compute the count/density. Used when \"-objfuntype\" is \"generic\".", "1" },
	{ "-gofrac", "double", "1", "Fraction of GO_k Terms to Keep", "Used for GO similarity (\"-go_k\"). It is the fraction of GO term ocurrences corresponding to the least frequent terms to be kept.", "1" },
	{ "-nodecweights", "vector", "4 .1 .25 .5 .15", "Weights of Node Density Measure", "Weights w of the Node count/density measure. They are automatically scaled to 1.", "1" },
	{ "-edgecweights", "vector", "4 .1 .25 .5 .15", "Weights of Edge Density Measure", "Weights w of the Edge count/density measure. They are automatically scaled to 1.", "1" },
	{ "-goweights", "vector", "1 1", "Weight and Measure of GO Measures", "Specifies the maximum GO measure and the weight of each one.", "1" },
	{ "-wecnodesim", "string", "graphletlgraal", "Weighted Edge Coverage Node Pair Similarity", "Node pair similarity used to weight the edges in the WEC measure. The edges are weighted by taking the average of the scores of an edge's two ending nodes using some node similarity measure which can be different from the default node sim measure.", "1" },
	{ "-wavenodesim", "string", "nodec", "Weighted Average Node Pair Similarity", "Node pair similarity to use when emulating WAVE.", "1" },
	{ "-maxGraphletSize", "double", "", "Maximum Graphlet Size", "Chooses the maximum size of graphlets to use. Saves human_gdv and yeast_gdv files ending with the given maximum graphlet size in order to distinguish between different-sized graphlets (e.g. human_gdv4.txt and yeast_gdv4.txt, for maximum graphlet size of 4).", "0" },
	{ "-simFile", "string", "", "Similarity File", "Specify an external three columnn (node from G1, node from  G2, similarity) file. These will be given weight according to the -esim argument.", "1" },
        { "-ewecFile", "string", "", "egdvs file", "egdvs output file produced by GREAT", "1"},
        { "-detailedreport", "bool", "false", "Detailed Report", "If false, initialize only basic measures and any measure necessary to run SANA.", "1" },
	{ "End Further Weight Specification. Combine with \"-method x -objfuntype y\"", "", "banner", "", "", "0" },
	//--------------END Further details of \"-method x -objfuntype y\"-------------------

	//------------------------------------MODE-------------------------------------------
	{ "", "", "banner", "", "Mode", "0" },
	{ "-mode", "string", "normal", "Mode", "Runs SANA in a specified mode. Arguments for this option are: \"cluster\", \"exp\", \"param\", \"alpha\", \"dbg\", \"normal\", \"analysis\", \"similarity\".", "0" },
	{ "End Mode", "", "banner", "", "", "0" },
	//----------------------------------END MODE-----------------------------------------

	//----------------------------------CLUSTER------------------------------------------
	{ "", "", "banner", "", "More options for \"-mode cluster\"", "0" },
	{ "-qmode", "string", "normal", "Queue Mode", "The \"Queue Mode\" when using SGE", "0" },
	{ "-qsuboutfile", "string", "", "Queue Mode Output File", "Output file when using SGE", "0" },
	{ "-qsuberrfile", "string", "", "Queue Error File", "Error file when using SGE", "0" },
	{ "-qsubscriptfile", "string", "", "Queue Script File", "Script file when using SGE", "0" },
	{ "-qcount", "double", "1", "Qsub Count", "When submitting to SGE, number of times to submit the same job.", "0" },
	{ "End More options for \"-mode cluster\"", "", "banner", "", "", "0" },
	//--------------------------------END CLUSTER----------------------------------------

	//--------------------------------EXPERIMENTAL---------------------------------------
	{ "", "", "banner", "", "More options for \"-mode exp\"", "0" },
	{ "-outfolder", "string", "", "Experimental Mode Output Folder", "The folder SANA will output results to during Experiment Mode.", "0" },
	{ "-experiment", "string", "", "Experimental Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on experiment mode.", "0" },
	{ "-collect", "bool", "false", "SGE Results Collection", "Collects results from a running or finished SGE job.", "0" },
	{ "-dbg", "bool", "false", "Debug Flag", "Each mode in SANA might have a different use for the dbg flag (e.g., in experiments, if -dbg is found, the runs are not actually sent to the cluster, only printed).", "0" },
	{ "-local", "bool", "false", "Submit Jobs Locally", "Used with SGE. Will not submit jobs to SGE, but jobs will be run locally instead.", "0" },
	{ "-updatecsv", "bool", "false", "Update CSV without re-running", "Used with expermient mode. If the data has already been collected, instead of re-evaluating all the alignments, it loads up the data and updates the csv output file.", "0" },
	{ "End More options for \"-mode exp\"", "", "banner", "", "", "0" },
	//------------------------------END EXPERIMENTAL-------------------------------------

	//----------------------------PARAMETER ESTIMATION-----------------------------------
	{ "", "", "banner", "", "More options for \"-mode param\"", "0" },
	{ "-paramestimation", "string", "", "Parameter Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode", "0" },
	{ "-submit", "bool", "false", "Print Jobs Instead of Submitting", "Used with Debugging Mode for SGE. If false, SGE jobs will not be submitted to queues, but will be printed to console instead.", "0" },
	{ "End More options for \"-mode param\"", "", "banner", "", "", "0" },
	//--------------------------END PARAMETER ESTIMATION---------------------------------

	//------------------------------ALPHA ESTIMATION-------------------------------------
	{ "", "", "banner", "", "More options for \"-mode alpha\"", "0" },
	{ "-alphaestimation", "string", "", "Alpha Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode", "0" },
	{ "End More options for \"-mode alpha\"", "", "banner", "", "", "0" },
	//----------------------------END ALPHA ESTIMATION-----------------------------------

	//-----------------------------------DEBUG-------------------------------------------
	//---------------------------------END DEBUG-----------------------------------------

	//----------------------------------NORMAL-------------------------------------------
	//--------------------------------END NORMAL-----------------------------------------

	//---------------------------------ANALYSIS------------------------------------------
	{ "", "", "banner", "", "More options for \"-mode analysis\"", "0" },
	{ "-alignFormat", "integerD", "0", "Alignfile Format", "Used in Analysis Mode \"-mode analysis\". alignFormat can be 1, 2, 3, or 4 which mean the following:\n1: sana.out format\n2. edge list format\n3. partial edge list format\n4. mapping format (old sana.out one line).", "0" },
	{ "-alignFile", "string", "", "Alignment File Analysis Mode", "Used with \"-mode analysis\" to specify which pre-existing alignment file is being analyzed. Allowed values are 1=sana.out; 2=edge list; 3=partial edge list; 4=mapping (one line).", "0" },
	{ "End More options for \"-mode analysis\"", "", "banner", "", "", "0" },
	//-------------------------------END ANALYSIS----------------------------------------

	//---------------------------------SIMILARITY----------------------------------------
	{ "", "", "banner", "", "More options for \"-mode similarity\"", "0" },
	{ "-simFormat", "double", "1", "Similarity File Format", "Used in Similarity Mode \"-mode similarity\" and with \"-objfuntype -esim\". Allowed values are 1=node names; 0=node integers numbered as in LEDA .gw format.", "0" },
	{ "End More options for \"-mode similarity\"", "", "banner", "", "", "0" },
	//-------------------------------END SIMILARITY--------------------------------------
	
	 //---------------------------------UNDEFINED----------------------------------------
	{ "-balance", "string", "", "TITLE", "DESCRIPTION", "0" },
	{ "-sec", "double", "0", "TITLE", "DESCRIPTION", "0" },
	{ "-nodes-have-types", "bool", "", "TITLE", "DESCRIPTION", "0" },
	{ "-balance-all", "bool", "", "TITLE", "DESCRIPTION", "0" },
	{ "-usingIterations", "bool", "", "TITLE", "DESCRIPTION", "0" },
	{ "-add-hill-climbing", "bool", "", "TITLE", "DESCRIPTION", "0" },
	{ "-gocov", "double", "", "Go Coverage", "DESCRIPTION", "0" }
	 //-------------------------------END UNDEFINED--------------------------------------
};

void validateAndAddArguments(){
	for(uint i = 1; i < supportedArguments.size(); ++i){
		if(supportedArguments[i][2] != "banner"){
			if(supportedArguments[i][0] == "") {
				cerr << "Option: #" << i+1 << " is empty for the Option field. Please specify the Option name in the supportedArguments.cpp file.";
				exit(1);
			}

			if(supportedArguments[i][1] == "") {
				cerr << "Option: #" << i+1 << " is empty for the Type field. Please specify the option Type in the supportedArguments.cpp file.";
                        	exit(1);
			}

			/*if(supportedArguments[i][2] == "") {
				cerr << "Option: #" << i+1 << " is empty for the Default Type field. Please specify the Default Type in the supportedArguments.cpp file.";
                	        exit(1);
			}*/

			if(supportedArguments[i][3] == "") {
				cerr << "Option: #" << i+1 << " is empty for the Title field. Please specify the Title in the supportedArguments.cpp file.";
                        	exit(1);
			}

			if(supportedArguments[i][4] == "") {
				cerr << "Option: #" << i+1 << " is empty for the Description field. Please write a description in the supportedArguments.cpp file.";
                        	exit(1);
			}

			if(supportedArguments[i][5] == "") {
			}

			if(supportedArguments[i][1] == "string" || supportedArguments[i][1] == "integerS" || supportedArguments[i][1] == "doubleS")
                	        stringArgs.push_back(supportedArguments[i][0]);

			else if(supportedArguments[i][1] == "double" || supportedArguments[i][1] == "integerD")
				doubleArgs.push_back(supportedArguments[i][0]);

			else if(supportedArguments[i][1] == "bool")
                        	boolArgs.push_back(supportedArguments[i][0]);

			else if(supportedArguments[i][1] == "vector")
                	        vectorArgs.push_back(supportedArguments[i][0]);

			else{
				cerr << "Option: " << supportedArguments[i][0] << " is type " << supportedArguments[i][1] << " which is not supported. Please check to make sure this option has a correct type.";
				exit(1);
			}

		}	
	}
}

void printAllArgumentDescriptions()
{
	ifstream helpOutput;
	helpOutput.open("./src/arguments/helpOutput");
	string line;
	getline(helpOutput, line);
	while (line != "OPTIONS, ARGS, AND DESCRIPTIONS GO HERE.")
	{
		cout << line << endl;
		getline(helpOutput, line);
	}

	for (auto it = supportedArguments.begin(); it != supportedArguments.end(); it++)
	{
		cout << printItem(*it);
	}

	/*string allOutput = "";
 * 	while(!helpOutput.eof()){
 *		getline(helpOutput, line);
 * 		allOutput += line + " ";
 * 	}
 *
 * 	while(!allOutput.size() > 80){                                 //In case a way is found to automatically generate the width of the helpOutput file in a non-complicated way, this commented code will
 * 		int space_location = 80;                               //extract a line from the helpOutput file at a specified width and print to the console. Change the constant 80 to some variable.
 * 		while(allOutput[space_location] != ' ')
 * 			space_location--;
 * 		cout << '\n' << allOutput.substr(0, space_location);
 * 		allOutput = allOutput.substr(space_location+1, allOutput.size() - space_location);
 * 	}
 * 	cout << '\n' << allOutput;*/
	getline(helpOutput, line);
	cout << line;
	while (!helpOutput.eof())
	{
		getline(helpOutput, line);
		cout << endl << line;
	}

	helpOutput.close();
}

string printItem(const array<string, 6> &item)
{
	ostringstream toReturn;
	toReturn << '\n';
	if (item[2] == "banner")
		if (item[0] == "") {
			toReturn << '\n';
			int width = 80 - item[4].size();
			int dash_width = width / 2;
			string dashes = "";
			for (int i = 0; i < dash_width - 1; i++)
				dashes += '-';
			toReturn << dashes << ' ' << item[4];
			if (width % 2)
				dashes += '-';
			toReturn << ' ' << dashes;
		}
		else {
			int width = 80 - item[0].size();
			int dash_width = width / 2;
			string dashes = "";
			for (int i = 0; i < dash_width - 1; i++)
				dashes += '-';
			toReturn << dashes << ' ' << item[0];
			if (width % 2)
				dashes += '-';
			toReturn << ' ' << dashes;
			toReturn << '\n';
		}
	else
	{
		toReturn << ' ' << setw(17) << left << item[0] << setw(8) << left << item[1] << setw(16) << left;
		if (item[2].length() == 0)
			toReturn << "\"\"";
		else
			toReturn << item[2];
		toReturn << item[3] << '\n';
		if (hasNewLines(item[4])) {
			toReturn << formatWithNewLines(item[4]);
		}
		else
			toReturn << formatDescription(item[4]);
	}
	return toReturn.str();
}

string formatDescription(string description)
{
	ostringstream toReturn;
	int end = 38;
	while (description.length() > 38)
	{
		while (description[end] != ' ')
		{
			end--;
		}
		toReturn << setw(42) << left << "" << description.substr(0, end) + '\n';
		description = description.substr(end + 1, description.length());
		end = 38;
	}
	toReturn << setw(42) << left << "" << description;
	return toReturn.str();
}

bool hasNewLines(const string &item4) {
	return (item4.find_first_of('\n') != string::npos);
}

string formatWithNewLines(const string &item4) {
	string toReturn = "";
	toReturn += formatDescription(item4.substr(0, item4.find_first_of('\n'))) + '\n';
	string textWithNewLine = item4.substr(item4.find_first_of('\n') + 1, item4.size() - item4.find_first_of('\n') + 1);
	while (textWithNewLine.find('\n') != string::npos) {
		toReturn += formatDescription(textWithNewLine.substr(0, textWithNewLine.find_first_of('\n'))) + '\n';
		textWithNewLine = textWithNewLine.substr(textWithNewLine.find_first_of('\n') + 1, textWithNewLine.size() - textWithNewLine.find_first_of('\n') + 1);
	}
	toReturn += formatDescription(textWithNewLine);
	return toReturn;
}
