#include "supportedArguments.hpp"

using namespace std;

vector<string> stringArgs;
vector<string> doubleArgs;
vector<string> boolArgs;
vector<string> doubleVectorArgs;
vector<string> stringVectorArgs;

//This file contains every argument supported by SANA contained basically inside an array, each element in the array contains 6 fields.
//A Description of each field:
// "Option": name of the argument, always begins with a dash.
// "Type": datatype/datastructure that is expected as an argument for that option.
// "Default": default value if none is given on the command line
// "Title": longer English title (used only by "./sana --help")
// "Description": arbitrarily long description for the option.
// "0/1": Boolean: can this option be accessed from other interfaces controlling SANA (when SANA is NOT run from a shell)
//
// Each line below must start with a "{" and end with "}," (modulo whitespace), since we use a script to convert this to CSV.

vector<array<string, 6>> supportedArguments = {
    { "Option", "Type", "Default", "Title", "Description", "0/1" },
    //-----------------------------------GENERAL-----------------------------------------
    { "", "general", "banner", "", "General Options", "0" },
    { "-g1", "string", "yeast", "Network 1", "First nerwork (smaller one). Requirement: An alignment file must exist inside the networks directory which matches the name of the specified species.", "0" },
    { "-g2", "string", "human", "Network 2", "Second (larger in number of nodes) network.  Requirement: An alignment file must exist inside the networks directory which matches the name of the specified species.", "0" },
    { "-fg1", "string", "yeast", "Network 1", "Initializes the network G1 with an external file. Make sure that the name and path of the file exists.", "1" },
    { "-fg2", "string", "human", "Network 2", "Initializes the network G2 with an external file. Make sure that the name and path of the file exists. The network of G2 should have more nodes than G1.", "1" },
    { "-o", "string", "sana", "Output File basename", "Specifies the basename of output file; actual output files will append (.out, .align, etc) to this.", "0" },
  { "-localScoresFile", "string", "sana", "Local Scores File basename", "Specifies the basename of the local score file; actual output files will append (.out, .align, etc) to this.", "0"},
    { "-t", "double", "5", "Runtime in minutes", "The number of minutes to run SANA. Must be non-zero, no upper limit.", "1" },
    { "-pathmap1", "intS", "", "Path Map G1", "Allows mapping a path in G1 to an edge in G2, as if the path were a single edge in G2. Implemented by raising the adjacency list to this power (an integer).", "1" },
    { "-pathmap2", "intS", "", "Path Map G2", "Maps a path in G2 to an edge in G1, as if the path were a single edge in G1.", "1" },
    { "-eval", "string", "", "Evaluate Existing Alignment", "Takes an existing alignment, evaluates it, and records the results to sana.out or the specified output file.", "1" },
    { "-startalignment", "string", "", "Starting Alignment", "File containing the starting alignment (in the format outputted by SANA). Some methods allow this option, while the rest start with random alignments.", "1" },
    { "-truealignment", "string", "", "True Alignment", "Alignment file containing the \"true\" alignment. This is used to evaluate the NC measure. In its absence, NC assumes that the true alignment is the identity (the node with index i in G1 is mapped to the node with index i in G2). In any case, NC is expressed as the fraction of nodes in the smaller network aligned correctly.", "0" },
    { "-rewire1", "double", "0", "Add rewiring noise to G1", "If set greater than 0, the corresponding fraction of edges in G1 is randomly rewired.", "0" },
    { "-rewire2", "double", "0", "Add rewiring noise to G2", "If set greater than 0, the corresponding fraction of edges in G2 is randomly rewired.", "0" },
    { "End General Options", "", "banner", "", "", "0" },
    //---------------------------------END GENERAL---------------------------------------

    //------------------------------------METHOD-----------------------------------------
    { "", "methods", "banner", "", "Method", "0" },
    { "-method", "string", "sana", "Method", "Sets the algorithm that performs the alignment. NOTE: All methods except \"sana\" call an external algorithm written by other authors. \nPossible aligners are: \"lgraal\", \"hubalign\", \"sana\", \"wave\", \"random\", \"tabu\", \"dijkstra\", \"netal\", \"mi-graal\", \"ghost\", \"piswap\", \"optnetalign\", \"spinal\", \"great\", \"natalie\", \"gedevo\", \"greedylccs\", \"magna\", \"waveSim\", \"none\", and \"hc\".", "1" },
    { "End Method", "", "banner", "", "", "0" },
    //----------------------------------END METHOD---------------------------------------

    //------------------------------------SANA-------------------------------------------
    { "", "method-sana", "banner", "", "Additional options to consider when method is \"sana\" (the default)", "0" },
    { "-tinitial", "dblS", "auto", "Initial Temperature", "Starting temperature of the simulated annealing. \"auto\" means calculate optimal starting temperature (requires extra CPU time).", "1" },
    { "-tdecay", "dblS", "auto", "Rate of Decay", "Exponential decay parameter of the temperature schedule. \"auto\" means calculate optimal starting temperature (requires extra CPU time).", "1" },
    { "-schedulemethod", "string", "auto", "Method to compute temperature schedule parameters automatically", "Specify the method to use set up the initial temperature and decay rate when they are set to 'auto'", "0" },
    { "-combinedScoreAs", "string", "sum", "Score Combo Method", "If multiple objectives are specified, this specifies how to combine them. Choices are: sum, product, inverse, max, min, maxFactor.", "1" },
    { "-dynamictdecay", "bool", "0", "Dynamically control temperature decay", "Whether or not tdecay is set to auto, this Boolean specifies if we should dynamically adjust the temperature schedule as the anneal progresses. Gives potentially better results than fixed decay rate.", "1" },
    { "-lock", "string", "", "Node-to-Node Locking", "Specify a two column file of node pairs that are locked in the alignment.", "0" },
    { "-lock-same-names", "bool", "false", "Node-to-Node Locking", "Locks nodes with same name together.", "0" },
    { "-seed", "double", "RANDOM", "Random Seed", "Serves as a random seed in SANA.", "0" },
    { "-restart", "bool", "false", "(DEPRECATED)Restart Scheme", "(DEPRECATED)Active the restart scheme in SANA.", "0" },
    { "-scheduleOnly", "bool", "false", "Temperature schedule only", "Compute temperature schedule and exit", "0" },
    { "-multi-iteration-only", "bool", "false", "Multi-Pairwise Iteration", "Skip all analysis for a normal iteration, just do the anneal", "0"},


    { "-tnew", "double", "3", "(DEPRECATED)Restart Scheme t1", "(DEPRECATED)Parameter t1 of the restart scheme, in minutes", "0" },
    { "-iterperstep", "double", "10000000", "(DEPRECATED)Restart Scheme N", "(DEPRECATED)Parameter N of the restart scheme.", "0" },
    { "-numcand", "double", "3", "(DEPRECATED)Restart Scheme K", "(DEPRECATED)Parameter K of the restart scheme.", "0" },
    { "-tcand", "double", "1", "Restart Scheme t2(DEPRECATED)", "(DEPRECATED)Parameter t2 of the restart scheme, in minutes", "0" },
    { "-tfin", "double", "3", "Restart Scheme t3(DEPRECATED)", "(DEPRECATED)Parameter t3 of the restart scheme, in minutes", "0" },
    { "-use-score-based-schedule", "bool", "false", "Use score based regression", "Enable to use a pbad based linear regression", "0" },
    { "END Additional options to consider when method is \"sana\" (the default)", "", "banner", "", "", "0" },
    //----------------------------------END SANA-----------------------------------------

    //------------------------------------TABU-------------------------------------------
    { "", "method-tabu", "banner", "", "Additional options to consider when method is \"tabu\"", "0" },
    { "-ntabus", "double", "300", "(DEPRECATED)TABU Argument", "(DEPRECATED)Argument for the TABU search function.", "0" },
    { "-nneighbors", "double", "50", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU.", "0" },
    { "-nodetabus", "bool", "false", "(DEPRECATED)See TABU", "(DEPRECATED)See TABU.", "0" },
    { "END Additional options to consider when method is \"tabu\"", "", "banner", "", "", "0" },
    //----------------------------------END TABU-----------------------------------------

    //----------------------------------Dijkstra-----------------------------------------
    { "", "method-djikstra", "banner", "", "Additional options to consider when method is \"dijkstra\"", "0" },
    { "-dijkstradelta","double","0","Dijkstra Randomness Amount","When picking the next edge to cross when adding a new node in the seed-and-extend Dijkstra algorithm, -dijkstradelta will set the width of the range of node similarity values that should be considered equal. Larger values inject more randomness; zero means \"pick best node according to current measure\".", "0" },
    { "END Additional options to consider when method is \"dijkstra\"", "", "banner", "", "", "0" },
    //---------------------------------END Dikstra---------------------------------------

    //-----------------------------------LGRAAL------------------------------------------
    { "", "method-lgraal", "banner", "", "Additional options to consider when method is \"lgraal\"", "0" },
    { "-lgraaliter", "double", "1000", "LGRAAL Iteration", "Number of LGRAAL Iterations. This option is passed along to the external LGRAAL program.", "0" },
    { "End Additional options to consider when method is \"lgraal\"", "", "banner", "", "", "0" },
    //---------------------------------END LGRAAL----------------------------------------

    //---------------------------Objective Function Type---------------------------------
    { "", "objectives-type", "banner", "", "Objective Function", "0" },
    { "-objfuntype", "string", "generic", "Objective Function Type", "Acceptable arguments are \"alpha\", \"beta\", or \"generic\". Make sure the chosen argument of \"-objectfuntype\" also matches with the chosen argument for \"-method\".", "1" },
    { "End Objective Function", "", "banner", "", "", "0" },
    //-------------------------END Objective Function Type-------------------------------

    //--------------------Objective Function Type Specifications-------------------------
    { "", "objectives-simple", "banner", "", "Objective Function Specification and Weight. Combine with \"-objfuntype x\"", "0" },
    { "-ec", "double", "0", "Weight of EC", "The weight of the Edge Coverage (aka Edge Conservation or Edge Correctness) in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-s3", "double", "1", "Weight of S3", "The weight of the Symmetric Substructer Score in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-js", "double", "1", "Weight of JS", "The weight of the Jaccard Similarity in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-ics", "double", "0", "Weight of ICS", "The weight of the Induced Conserved Structure in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-importance", "double", "0", "Weight of Importance", "HubAlign's objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-graphlet", "double", "0", "Weight of Graphlet Similarity.", "The weight of the Graphlet Objective Function as defined in the original GRAAL paper (2010). Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-graphletlgraal", "double", "0", "Weight of Graphlet Similarity (LGRAAL)", "The weight of LGRAAL's objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-esim", "dbl_vec", "0", "External Similarity Weights", "An integer followed by that many weights, specifying objective function weights for external similarity files (must be the same integer as given to -simFile and -simFormat).", "1" },
    { "-wec", "double", "0", "Weight of WEC.", "Weight of the weighted edge coverage. To be used when \"-objfuntype\" is \"generic\". If non-zero, must specify how to weigh the edge using -wecnodesim.", "1" },
    { "-ewec", "double", "0", "External Weighted Edge Similarity", "The weighted of the external edge similarity file.", "1" },
    { "-ed", "double", "0", "Weight of ED", "The weight of the Edge Difference in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-er", "double", "0", "Weight of ER", "The weight of the Edge Ratio in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-nc", "double", "0", "Weight of NC", "This weight of Node Correctness in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-tc", "double", "0", "Weight of TC", "The weight of Triangle Correctness in the objetive function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-spc", "double", "0", "Shortest Path Conservation", "Objective function to minimize shortest path difference between pairs of nodes in G1 and G2. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-nodec", "double", "0", "Weight of Node Count", "The weight of the Local Node Count objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-noded", "double", "0", "Weight of Node Density", "The weight of the Local Node Density objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-edgec", "double", "0", "Weight of Edge Count", "The weight of the Local Edge Count objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-edged", "double", "0", "Weight of Edge Density", "The weight of the Local Edge Density objective function. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-alpha", "double", "0", "alpha=0 means toplogy only; alpha=1 means sequence only.", "Trade off between biological (alpha = 1) and topological (alpha = 0) measures. Range: [0, 1]. Used when \"-objfuntype\" is \"alpha\".", "1" },
    { "-beta", "double", "0", "Like -alpha, but values are normalized first.", "Same as alpha but with topological and biological scores balanced according to size. Range: [0, 1]. Used when \"-objfuntype\" is \"beta\".", "1" },
    { "-sequence", "double", "0", "Weight of Sequence Similarity", "The weight of the Sequence Similarity function. Used when \"-objfuntype\" is \"generic\".", "1" },
        { "-graphletnorm", "double", "0", "similarities between the normalization of 2 GDV vectors", "objective function based on the ratio between vector elements. Used when \"-objfuntype\" is \"generic\".", "1" },

    { "-go_k", "double", "0", "k-common GO terms", "Objective function based on having up to k GO terms in common. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-graphletcosine", "double", "0", "Similarity of cosine between GDV vectors", "Objective function based on the cosine angle between graphlet degree vectors. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-topomeasure", "string", "", "Topological Measure", "Topological component of the scoring function. Used when \"-objfuntype\" is either \"alpha\" or \"beta\".", "1" },
    { "", "", "banner", "", "Multiple Alignment Objectives (must compile SANA with \"make multi\")", "0" },
    { "-mec", "double", "0", "Weight of MEC", "The weight of the Multiple Edge Coverage in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-ses", "double", "0", "Weight of SES", "The weight of the Squared Edge Score in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-ee", "double", "0", "Weight of EE", "The weight of the Edge Exposure Score in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-ms3", "double", "0", "Weight of MS3", "The weight of the Multi Symmetric Substructer Score in the objective function. To be used when \"-objfuntype\" is \"generic\".", "1" },
    { "-ms3_type", "double", "0", "type of MS3", "some description", "0" },
    { "-ms3_numer", "string", "default", "numerator MS3", "Chioces are: default, ra_i, la_i, la_global, ra_global", "0" },
    { "-ms3_denom", "string", "default", "denominator MS3", "Choices are: default, rt_i, ee_i, ee_global, rt_global", "0" },
    { "End Objective Function Specification and Weight. Combine with \"-objfuntype x\"", "", "banner", "", "", "0" },
    //-------------------END Objective Function Tyoe Specifications----------------------

    //-----------------Further details of \"-method x -objfuntype y\"--------------------
    { "", "objectives-complex", "banner", "", "Further Weight Specification. Combine with \"-method x -objfuntype y\"", "0" },
    { "-wrappedArgs", "string", "", "Wrapper Function Arguments", "Arguments to pass verbatim to wrapped methods.", "0" },
    { "-maxDist", "double", "1", "Radial Distance from Node", "When using nodec, edgec, noded, or edged, the radial distance region over which to compute the count/density. Used when \"-objfuntype\" is \"generic\".", "1" },
    { "-gofrac", "double", "1", "Fraction of GO_k Terms to Keep", "Used for GO similarity (\"-go_k\"). It is the fraction of GO term ocurrences corresponding to the least frequent terms to be kept.", "1" },
    { "-nodecweights", "dbl_vec", "4 .1 .25 .5 .15", "Weights of Node Density Measure", "Weights w of the Node count/density measure. They are automatically scaled to 1.", "1" },
    { "-edgecweights", "dbl_vec", "4 .1 .25 .5 .15", "Weights of Edge Density Measure", "Weights w of the Edge count/density measure. They are automatically scaled to 1.", "1" },
    { "-goweights", "dbl_vec", "1 1", "Weight and Measure of GO Measures", "Specifies the maximum GO measure and the weight of each one.", "1" },
    { "-wecnodesim", "string", "graphletlgraal", "Weighted EC based on mean Node Pair Similarity", "Node pair similarity used to weight the edges in the WEC measure. The edges are weighted by taking the average of the scores of an edge's two ending nodes using some node similarity measure which can be different from the default node sim measure.", "1" },
    { "-wavenodesim", "string", "nodec", "Weighted Average Node Pair Similarity", "Node pair similarity to use when emulating WAVE.", "1" },
    { "-maxGraphletSize", "double", "", "Maximum Graphlet Size", "Chooses the maximum size of graphlets to use. Saves human_gdv and yeast_gdv files ending with the given maximum graphlet size in order to distinguish between different-sized graphlets (e.g. human_gdv4.txt and yeast_gdv4.txt, for maximum graphlet size of 4).", "0" },
    { "-simFile", "str_vec", "0", "External Similarity Filenames", "An integer (same integer as given to -esim and -simFormat) followed by that many filenames, specifying external three-columnn (node from G1, node from  G2, similarity) similarities. The similarities in the 3rd column will be normalized to be in [0,1]. These simFiles will be given weight according to the -esim argument.", "1" },
        { "-ewecFile", "string", "", "egdvs file", "egdvs output file produced by GREAT", "1"},
        { "-detailedreport", "bool", "false", "Detailed Report", "If false, initialize only basic measures and any measure necessary to run SANA.", "1" },
    { "End Further Weight Specification. Combine with \"-method x -objfuntype y\"", "", "banner", "", "", "0" },
    //--------------END Further details of \"-method x -objfuntype y\"-------------------

    //------------------------------------MODE-------------------------------------------
    { "", "mode", "banner", "", "Mode", "0" },
    { "-mode", "string", "normal", "Mode", "Runs SANA in a specified mode. Arguments for this option are: \"cluster\", \"exp\", \"param\", \"alpha\", \"dbg\", \"normal\", \"analysis\", \"similarity\", \"pareto\".", "0" },
    { "End Mode", "", "banner", "", "", "0" },
    //----------------------------------END MODE-----------------------------------------

    //----------------------------------CLUSTER------------------------------------------
    { "", "mode-cluster", "banner", "", "More options for \"-mode cluster\"", "0" },
    { "-qmode", "string", "normal", "Queue Mode", "The \"Queue Mode\" when using SGE", "0" },
    { "-qsuboutfile", "string", "", "Queue Mode Output File", "Output file when using SGE", "0" },
    { "-qsuberrfile", "string", "", "Queue Error File", "Error file when using SGE", "0" },
    { "-qsubscriptfile", "string", "", "Queue Script File", "Script file when using SGE", "0" },
    { "-qcount", "double", "1", "Qsub Count", "When submitting to SGE, number of times to submit the same job.", "0" },
    { "End More options for \"-mode cluster\"", "", "banner", "", "", "0" },
    //--------------------------------END CLUSTER----------------------------------------

    //--------------------------------EXPERIMENTAL---------------------------------------
    { "", "mode-experimental", "banner", "", "More options for \"-mode exp\"", "0" },
    { "-outfolder", "string", "", "Experimental Mode Output Folder", "The folder SANA will output results to during Experiment Mode.", "0" },
    { "-experiment", "string", "", "Experimental Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on experiment mode.", "0" },
    { "-collect", "bool", "false", "SGE Results Collection", "Collects results from a running or finished SGE job.", "0" },
    { "-dbg", "bool", "false", "Debug Flag", "Each mode in SANA might have a different use for the dbg flag (e.g., in experiments, if -dbg is found, the runs are not actually sent to the cluster, only printed).", "0" },
    { "-local", "bool", "false", "Submit Jobs Locally", "Used with SGE. Will not submit jobs to SGE, but jobs will be run locally instead.", "0" },
    { "-updatecsv", "bool", "false", "Update CSV without re-running", "Used with expermient mode. If the data has already been collected, instead of re-evaluating all the alignments, it loads up the data and updates the csv output file.", "0" },
    { "End More options for \"-mode exp\"", "", "banner", "", "", "0" },
    //------------------------------END EXPERIMENTAL-------------------------------------

    //----------------------------PARAMETER ESTIMATION-----------------------------------
    { "", "mode-parameter", "banner", "", "More options for \"-mode param\"", "0" },
    { "-paramestimation", "string", "", "Parameter Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode", "0" },
    { "-submit", "bool", "false", "Print Jobs Instead of Submitting", "Used with Debugging Mode for SGE. If false, SGE jobs will not be submitted to queues, but will be printed to console instead.", "0" },
    { "End More options for \"-mode param\"", "", "banner", "", "", "0" },
    //--------------------------END PARAMETER ESTIMATION---------------------------------

    //------------------------------ALPHA ESTIMATION-------------------------------------
    { "", "mode-alpha", "banner", "", "More options for \"-mode alpha\"", "0" },
    { "-alphaestimation", "string", "", "Alpha Estimation Mode", "If any of the arguments -experiment, -paramestimation, or -alphaestimation is present, then the corresponding experiment is executed. See below for a description on parameter estimation mode", "0" },
    { "End More options for \"-mode alpha\"", "", "banner", "", "", "0" },
    //----------------------------END ALPHA ESTIMATION-----------------------------------

    //-----------------------------------DEBUG-------------------------------------------
    //---------------------------------END DEBUG-----------------------------------------

    //----------------------------------NORMAL-------------------------------------------
    //--------------------------------END NORMAL-----------------------------------------

    //---------------------------------ANALYSIS------------------------------------------
    { "", "-mode-analysis", "banner", "", "More options for \"-mode analysis\"", "0" },
    { "-alignFormat", "intD", "0", "Alignfile Format", "Used in Analysis Mode \"-mode analysis\". alignFormat can be 1, 2, 3, or 4 which mean the following:\n1: sana.out format\n2. edge list format\n3. partial edge list format\n4. mapping format (old sana.out one line).", "0" },
    { "-alignFile", "string", "", "Alignment File Analysis Mode", "Used with \"-mode analysis\" to specify which pre-existing alignment file is being analyzed. Allowed values are 1=sana.out; 2=edge list; 3=partial edge list; 4=mapping (one line).", "0" },
    { "End More options for \"-mode analysis\"", "", "banner", "", "", "0" },
    //-------------------------------END ANALYSIS----------------------------------------

    //---------------------------------SIMILARITY----------------------------------------
    { "", "mode-similarity", "banner", "", "More options for \"-mode similarity (which exports and saves the internal similarity matrix)\"", "0" },
    { "-simFormat", "int_vec", "0", "Similarity File Formats", "An integer (must be the same one used by -esim and -simFiles) followed by that many integer simFormats. Allowed values are 2=G1 doubles down by G2 doubles across matrix where node order corresponds to .gw files; 1=node names; 0=node integers numbered as in LEDA .gw format.", "0" },
    { "End More options for \"-mode similarity\"", "", "banner", "", "", "0" },
    //-------------------------------END SIMILARITY--------------------------------------

    //----------------------------------PARETO-------------------------------------------
    { "", "mode-pareto", "banner", "", "More options for \"-mode pareto\"", "0" },
    { "-paretoInitial", "intD", "10", "Initial Pareto Size", "Used in Pareto Mode \"-mode pareto\". This argument specifies the starting number of Alignments in the Pareto Front. All of the starting alignments are the same", "0" },
    { "-paretoCapacity", "intD", "200", "Capacity of Pareto Front", "Used in Pareto Mode \"-mode pareto\". The pareto front can potentially hold N (different scores) by X (possible values/precision of the datatype used (i.e. float or double). Therefore, if a capacity is not specified, after the billions of SANA iterations, there may be too many different alignments stored in memory.", "0" },
    {"-paretoIterations", "intD", "10000", "Iterations per Pareto Insertion", "Used in Pareto Mode \"-mode pareto\". Pareto mode operates on a copy of an alignment (which already exists in the Pareto front). The number of changes and swaps happens -paretoIterations number of times on this copy before attempting to add this new alignment to the Pareto front.", "0"},
    {"-paretoThreads", "string", "1", "Number of threads to run pareto mode currently.", "0"},
    { "End More options for \"-mode pareto\"", "", "banner", "", "", "0" },
    //--------------------------------END PARETO-----------------------------------------

     //---------------------------------UNDEFINED----------------------------------------
    { "-balance", "string", "", "TITLE", "DESCRIPTION", "0" },
    { "-sec", "double", "0", "TITLE", "DESCRIPTION", "0" },
    { "-bipartite", "bool", "", "TITLE", "DESCRIPTION", "0" },
    { "-multipartite", "double", "", "TITLE", "DESCRIPTION", "0" },
    { "-balance-all", "bool", "", "TITLE", "DESCRIPTION", "0" },
    { "-usingIterations", "bool", "", "TITLE", "DESCRIPTION", "0" },
    { "-add-hill-climbing", "bool", "", "TITLE", "DESCRIPTION", "0" },
    { "-netgo", "double", "", "NetGo", "DESCRIPTION", "0" }
     //-------------------------------END UNDEFINED--------------------------------------
};

void validateAndAddArguments(){
    for(uint i = 1; i < supportedArguments.size(); ++i){
        if(supportedArguments[i][2] != "banner"){
            if(supportedArguments[i][0] == "") {
                cout << "Option: #" << i+1 << " is empty for the Option field. Please specify the Option name in the supportedArguments.cpp file.\n";
                exit(1);
            }

            if(supportedArguments[i][1] == "") {
                cout << "Option: #" << i+1 << " is empty for the Type field. Please specify the option Type in the supportedArguments.cpp file.\n";
                            exit(1);
            }

            /*if(supportedArguments[i][2] == "") {
                cout << "Option: #" << i+1 << " is empty for the Default Type field. Please specify the Default Type in the supportedArguments.cpp file.";
                            exit(1);
            }*/

            if(supportedArguments[i][3] == "") {
                cout << "Option: #" << i+1 << " is empty for the Title field. Please specify the Title in the supportedArguments.cpp file.\n";
                            exit(1);
            }

            if(supportedArguments[i][4] == "") {
                cout << "Option: #" << i+1 << " is empty for the Description field. Please write a description in the supportedArguments.cpp file.\n";
                            exit(1);
            }

            if(supportedArguments[i][5] == "") {
            }

            if(supportedArguments[i][1] == "string" || supportedArguments[i][1] == "intS" || supportedArguments[i][1] == "dblS")
                            stringArgs.push_back(supportedArguments[i][0]);

            else if(supportedArguments[i][1] == "double" || supportedArguments[i][1] == "intD")
                doubleArgs.push_back(supportedArguments[i][0]);

            else if(supportedArguments[i][1] == "bool")
                boolArgs.push_back(supportedArguments[i][0]);
            else if(supportedArguments[i][1] == "dbl_vec" || supportedArguments[i][1] == "int_vec")
                doubleVectorArgs.push_back(supportedArguments[i][0]);
            else if(supportedArguments[i][1] == "str_vec")
                stringVectorArgs.push_back(supportedArguments[i][0]);
            else{
                cout << "Option: " << supportedArguments[i][0] << " is type " << supportedArguments[i][1] << " which is not supported. Please check to make sure this option has a correct type.\n";
                exit(1);
            }

        }
    }
}

void printAllArgumentDescriptions(unordered_set<string> help_args) {
    ifstream helpOutput;
    ifstream helpTutorial;

    helpOutput.open("./src/arguments/helpOutput");
    helpTutorial.open("./src/arguments/helpTutorial");
    
    string line;

    if (help_args.size() == 0){ // Checks if no extra flag is given, If none prints general helpOutput file
        while (!helpOutput.eof())
        {
            getline(helpOutput, line);
            if (line != "OPTIONS, ARGS, AND DESCRIPTIONS GO HERE."){
                cout << endl << line;
            }
        }
    }

    bool hasTutorial = false; // To check if tutorial file need to be printed
    bool printingArguments = false; // Checks if the help arguments ask for the banner being iterated over
    
    for (auto it = supportedArguments.begin(); it != supportedArguments.end(); it++)
    {
        auto argLine = *it;
        if (argLine == supportedArguments[0]){
            cout << printItem(argLine) << endl;
        }
        if (argLine[2] == "banner")
        {
            printingArguments = false;
            for (const auto& ele: help_args){
                if (!ele.compare(argLine[1]) || !ele.compare("all")){
                    printingArguments = true;
                    break;
                }
                else if(!ele.compare("tutorial")){
                    hasTutorial = true;
                }
            }
        }
        if (printingArguments == true)
        {
            cout << printItem(argLine) << endl;
        }
    }

    if (hasTutorial == true){ // if tutorial is asked for print tutorial
        while (!helpTutorial.eof())
        {
            getline(helpTutorial, line);
            if (line != "OPTIONS, ARGS, AND DESCRIPTIONS GO HERE."){
                cout << endl << line;
            }
        }
    }

    helpOutput.close();
    helpTutorial.close();
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
