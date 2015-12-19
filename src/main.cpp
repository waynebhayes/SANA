#include "arguments/ArgumentParser.hpp"
#include "arguments/supportedArguments.hpp"
#include "arguments/defaultArguments.hpp"
#include "arguments/modeSelector.hpp"
#include "randomSeed.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
    args.parseArgs(argc, argv, defaultArguments, true);

    if(args.doubles["-seed"] != 0) {
        setSeed(args.doubles["-seed"]);
    }

    args.writeArguments();

    Mode* mode = selectMode(args);
    mode->run(args);
    delete mode;

    return 0;
}
