#ifndef ARGUMENTPARSER_HPP
#define ARGUMENTPARSER_HPP
#include <string>
#include <map>
#include <vector>
using namespace std;


class ArgumentParser {

public:
    map<string, string> strings;
    map<string, double> doubles;
    map<string, bool> bools;
    map<string, vector<double> > doubleVectors;
    map<string, vector<string> > stringVectors;
    vector<string> originalArgv; //argv but in vector format

    ArgumentParser(
        const vector<string> &defStringArgs,
        const vector<string> &defDoubleArgs,
        const vector<string> &defBoolArgs,
        const vector<string> &defDoubleVectorArgs,
        const vector<string> &defStringVectorArgs);
    
    void parseArgs(int argc, char* argv[],
        vector<string> baseValueLines, bool readParamsFromFileInFirstArg);

    void writeArguments();

private:
    void initDefaultValues(
        const vector<string> &defStringArgs,
        const vector<string> &defDoubleArgs,
        const vector<string> &defBoolArgs,
        const vector<string> &defDoubleVectorArgs,
        const vector<string> &defStringVectorArgs);

    void initParsedValues(vector<string> vArg);

    /*
    collects the arguments from the terminal command, a list of base values for some arguments,
    and possibly additional arguments from a file (see below)
    into a more uniform vector of strings.

    readParamsFromFileInFirstArg: allows the user to have his own file with
    default values. If this option is set, the user can optionally put his
    file with default values as first argument. 
    
    Note: arguments in the optional file will override base values, and command line
    arguments will override any other value.

    Note: all arguments that are not collected here will be initialized to the default
    value: string arguments are set to "", double arguments to 0,
    bool arguments to false, and vector arguments to an empty vector.
    */
    static vector<string> getFullArgList(const vector<string>& argv, 
        const vector<string>& baseValues, bool readParamsFromFileInFirstArg);


};




#endif


