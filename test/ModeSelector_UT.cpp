#include "gtest/gtest.h"
#include "../src/arguments/ModeSelector.hpp"
#include "../src/arguments/ArgumentParser.hpp"
#include "../src/arguments/SupportedArguments.hpp"
#include <string.h>
#include <string>
#include <vector>

#include <iostream>

TEST(ModeSelector_UT, verifyClusterModeIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.bools["-qsub"] = true;
	Mode* mode = selectMode(args);
	EXPECT_EQ("ClusterMode", mode->getName());
	delete mode;
}

TEST(ModeSelector_UT, verifyExperimentIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.strings["-experiment"] = "test";
	Mode* mode = selectMode(args);
	EXPECT_EQ("Experiment", mode->getName());
	delete mode;
}

TEST(ModeSelector_UT, verifyParamiterEstimationIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.strings["-paramestimation"] = "test";
	Mode* mode = selectMode(args);
	EXPECT_EQ("ParameterEstimation", mode->getName());
	delete mode;
}

TEST(ModeSelector_UT, verifyAlphaEstimationIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.strings["-alphaestimation"] = "test";
	Mode* mode = selectMode(args);
	EXPECT_EQ("AlphaEstimation", mode->getName());
	delete mode;
}

TEST(ModeSelector_UT, verifyDebugModeIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	args.bools["-dbg"] = true;
	Mode* mode = selectMode(args);
	EXPECT_EQ("DebugMode", mode->getName());
	delete mode;
}

TEST(ModeSelector_UT, verifyNormalModeIsSelected) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
	Mode* mode = selectMode(args);
	EXPECT_EQ("NormalMode", mode->getName());
	delete mode;
}

