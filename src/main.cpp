#include "arguments/ArgumentParser.hpp"
#include "arguments/SupportedArguments.hpp"
#include "arguments/defaultArguments.hpp"
#include "arguments/ModeSelector.hpp"
#include "RandomSeed.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.parse(getArgumentList(argc, argv, defaultArguments, true));

	if(args.doubles["-seed"] != 0) {
		setSeed(args.doubles["-seed"]);
	}

	args.writeArguments();

	Mode* mode = selectMode(args);
	mode->run(args);
	delete mode;

	return 0;
}
