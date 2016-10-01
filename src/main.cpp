#include "arguments/ArgumentParser.hpp"
#include "arguments/supportedArguments.hpp"
#include "arguments/defaultArguments.hpp"
#include "arguments/modeSelector.hpp"
#include "utils/randomSeed.hpp"
#include <iostream>

using namespace std;


int main(int argc, char* argv[]) {
    if(argc == 1) {
        cerr << "Usage: ./sana [OPTION] [ARG(S)] [OPTION] [ARG(S)]...\n"
	     << "Try \'./sana --help\' or \'./sana -h\' for more information."
	     << endl;
	exit(0);
    }

    ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
    args.parseArgs(argc, argv, defaultArguments, true);

    if(args.doubles["-seed"] != 0) {
        setSeed(args.doubles["-seed"]);
    }
    system("hostname -f >&2");

    args.writeArguments();

    Mode* mode = selectMode(args);
    mode->run(args);
    delete mode;

    return 0;
}
