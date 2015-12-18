vector<string> stringArgs = {
"-g1", "-g2",
"-fg1", "-fg2",
"-method",
"-o",
"-eval",
"-T_initial", "-T_decay", 
"-startalignment",
"-truealignment",
"-wecnodesim", "-wavenodesim",
"-experiment",
"-paramestimation",
"-alphaestimation", "-alphafile"
};

vector<string> doubleArgs = {
"-ec", "-s3", "-wec",
"-importance", "-noded", "-edged",
"-graphlet", "-graphletlgraal",
"-sequence", "-go",
"-t",
"-rewire", 
"-alpha",
"-tnew", "-iterperstep", "-numcand", "-tcand", "-tfin",
"-seed",
"-qcount",
"-ntabus", "-nneighbors",
"-lgraaliter"
};

vector<string> boolArgs = {
"-dbg",
"-restart",
"-detailedreport",
"-qsub",
"-goavg",
"-nodetabus",
"-autoalpha",
"-submit"
};

vector<string> vectorArgs = {
"-nodedweights",
"-edgedweights",
"-goweights"
};