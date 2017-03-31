#include "gtest/gtest.h"
#include <string.h>
#include <string>
#include <vector>
#include "../src/arguments/ArgumentParser.hpp"
#include "../src/arguments/SupportedArguments.hpp"

TEST(ArgumentParser_UT, verifyConstructorWorksAsIntended) {
	ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);

	//Check that all strings were loaded and initialized
	for(int i = 0; !strcmp(stringArgs[i], ""); ++i) {
		std::string str = stringArgs[i];
		EXPECT_TRUE(args.strings.find(str) != args.strings.end());
		EXPECT_EQ(args.strings[str], "");
	}

	//Check that all doubles were loaded and initialized
	for(int i = 0; !strcmp(doubleArgs[i], ""); ++i) {
		std::string str = doubleArgs[i];
		EXPECT_TRUE(args.doubles.find(str) != args.doubles.end());
		EXPECT_EQ(args.doubles[str], 0.0);
	}

	//Check that all bools were loaded and initialized
	for(int i = 0; !strcmp(boolArgs[i], ""); ++i) {
		std::string str = boolArgs[i];
		EXPECT_TRUE(args.bools.find(str) != args.bools.end());
		EXPECT_FALSE(args.bools[str]);
	}

	//Check that all Vectors were loaded and initialized
	for(int i = 0; !strcmp(vectorArgs[i], ""); ++i) {
		std::string str = vectorArgs[i];
		EXPECT_TRUE(args.vectors.find(str) != args.vectors.end());
		EXPECT_EQ(args.vectors[str], vector<double> (0));
	}
}
