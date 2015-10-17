#ifndef ARGUMENTPARSER_HPP
#define	ARGUMENTPARSER_HPP
#include <string>
#include <map>
#include <vector>
using namespace std;


class ArgumentParser {

public:
	map<string, string> strings;
	map<string, double> doubles;
	map<string, bool> bools;
	map<string, vector<double> > vectors;

	//each array should have an empty entry ("") to denote the end
	ArgumentParser(char listStringArgs[][80], char listDoubleArgs[][80], char listBoolArgs[][80], char listVectorArgs[][80]);
	
	void parse(vector<string> vArg);
	void writeArguments();

};

/*
collects the arguments from the terminal, as well as from the different possible files with
default values (see below) into a more uniform vector of strings, which can be fed to the parse function

useFileWithDefaultValues: by default, string arguments are set to "", double arguments to 0,
bool arguments to false, and vector arguments to an empty vector. However, this option
allows to specify default values for the arguments in the file fileWithDefaultValues.
This file should contain a line for each argument.

readParamsFromFileInFirstArg: allows the user to have his own file with default values.
If this option is set, the user can optionally put his file with default values as first argument 
*/
vector<string> getArgumentList(int argc, char* argv[], bool useFileWithDefaultValues,
	string fileWithDefaultValues, bool readParamsFromFileInFirstArg);

/*
Similar as above but instead of fileWithDefaultValues, it receives
a vector containing the lines of that file as elements
*/
vector<string> getArgumentList(int argc, char* argv[],
	vector<string> defaultValues, bool readParamsFromFileInFirstArg);



#endif


