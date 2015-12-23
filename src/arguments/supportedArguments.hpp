vector<string> stringArgs = {
"-g1", "-g2",
"-fg1", "-fg2",
"-method",
"-mode",
"-o",
"-eval",
"-tinitial", "-tdecay", 
"-startalignment",
"-truealignment",
"-wecnodesim", "-wavenodesim",
"-experiment",
"-paramestimation",
"-objfuntype", //can be "generic", "alpha", "beta"
               //generic: use '-ec','-s3',...
               //alpha: use '-topmeasure' and sequence balanced with '-alpha'
               //beta: use '-topmeasure' and sequence balanced with '-beta'
"-topmeasure", //can be "ec", "s3", "wec"
               //used when '-objfuntype' is "alpha" or "beta"

//for submissions to the cluster
"-qmode", //mode of the submission (since the value of mode will be 'cluster')
//the next 3 arguments are only read from the command line,
//they should not appear in defaultArguments.hpp
"-qsuboutfile",
"-qsuberrfile",
"-qsubscriptfile",
};

vector<string> doubleArgs = {
//objective function weights. used when '-objfuntype' is "generic"
"-ec", "-s3", "-wec",
"-importance", "-noded", "-edged",
"-graphlet", "-graphletlgraal",
"-sequence", "-go",

"-alpha", //used when '-objfuntype' is "alpha"
"-beta", //used when '-objfuntype' is "beta"

"-t",
"-rewire", 
"-tnew", "-iterperstep", "-numcand", "-tcand", "-tfin",
"-seed",
"-qcount",
"-ntabus", "-nneighbors",
"-lgraaliter",
};

vector<string> boolArgs = {
"-restart",
"-detailedreport",
"-nodetabus",
"-submit",
"-dbg", //each mode might have a different use for the dbg flag
        //e.g., in experiments if -dbg is found, the runs are
        //not actually sent to the cluster, only printed
};

vector<string> vectorArgs = {
"-nodedweights",
"-edgedweights",
"-goweights"
};