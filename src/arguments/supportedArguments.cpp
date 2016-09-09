#include "supportedArguments.hpp"

vector<string> stringArgs = {
"-g1", "-g2",
"-fg1", "-fg2",
"-method",
"-mode",
"-o",
"-pathmap1", "-pathmap2",
"-eval",
"-tinitial", "-tdecay",
"-startalignment",
"-truealignment",
"-wecnodesim", "-wavenodesim",
"-experiment",
"-outfolder",
"-paramestimation",
"-objfuntype", //can be "generic", "alpha", "beta"
               //generic: use '-ec','-s3',...
               //alpha: use '-topmeasure' and sequence balanced with '-alpha'
               //beta: use '-topmeasure' and sequence balanced with '-beta'
"-topmeasure", //can be "ec", "s3", "wec"
               //used when '-objfuntype' is "alpha" or "beta"

//for submissions to the cluster
"-qmode", //mode of the submission (since the value of mode will be 'cluster')
//the next 3 arguments are read from the command line,
//they should not appear in defaultArguments.hpp
"-qsuboutfile",
"-qsuberrfile",
"-qsubscriptfile",
"-wrappedArgs",
"-lock",    // for locking mechanism
"-alignFile",			// Used in analysis mode
"-simFile",
"-score",
"-balance",
};

vector<string> doubleArgs = {
//objective function weights. used when '-objfuntype' is "generic"
"-ec", "-s3", "-wec",  "-spc","-sec",
"-importance", "-nodec", "-noded", "-edgec", "-edged", "-esim",
"-graphlet", "-graphletlgraal",
"-sequence", "-go",
"-graphletcosine",
"-maxDist", // used for -edged and -noded
"-alpha", //used when '-objfuntype' is "alpha"
"-beta", //used when '-objfuntype' is "beta"

"-t",
"-i",
"-rewire",
"-tnew", "-iterperstep", "-numcand", "-tcand", "-tfin",
"-seed",
"-gofrac", //for GO similarity, fraction of GO term ocurrences
           //corresponding to the least frequent terms to be kept
"-qcount",
"-ntabus", "-nneighbors",
"-lgraaliter",
"-alignFormat",				// Used in analysis mode
"-simFormat",				// Used in similarity mode
"-dijkstradelta"
};

vector<string> boolArgs = {
"-restart",
"-detailedreport",
"-nodetabus",
"-submit",
"-local",	// submits experiments locally
"-dbg", //each mode might have a different use for the dbg flag
        //e.g., in experiments if -dbg is found, the runs are
        //not actually sent to the cluster, only printed
"-collect", //for experiment mode, aggregate all the results
            //(use when all runs have finished)
"-updatecsv", //for expermient mode, if the data has already
             //been collected, instead of re-evaluating all the
             //alignments it loads up the data and updates the
             //csv output file
"-nodes-have-types", // Used when the nodes in inpupt graph have types
                    // Added for handling gene-miRNA networks, currently only functional with these networks
"-dynamictdecay"
};

vector<string> vectorArgs = {
"-nodecweights",
"-edgecweights",
"-goweights"
};
