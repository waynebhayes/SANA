#include "ModeSelector.hpp"

#include "../modes/ParameterEstimation.hpp"
#include "../modes/AlphaEstimation.hpp"
#include "../modes/DebugMode.hpp"
#include "../modes/NormalMode.hpp"
#include "../modes/ClusterMode.hpp"
#include "../modes/Experiment.hpp"

Mode* selectMode(ArgumentParser& args) {
	Mode* mode;

	if (args.bools["-qsub"]) {
		mode = new ClusterMode();
	} else if (not strEq(args.strings["-experiment"], "")) {
		mode = new Experiment();
	} else if (not strEq( args.strings["-paramestimation"], "")) {
		mode = new ParameterEstimation();
	} else if (not strEq(args.strings["-alphaestimation"], "")) {
		mode = new AlphaEstimation();
	} else if (args.bools["-dbg"]) {
		mode = new DebugMode();
	} else {
		mode = new NormalMode();
	}

	return mode;
}
