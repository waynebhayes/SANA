#include <iostream>
#include "ArgumentParser.hpp"
#include "../utils/utils.hpp"
#include "../utils/FileIO.hpp"
#include "defaultArguments.hpp"
#include "SupportedArguments.hpp"
using namespace std;

ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    if(argc == 1) {
        cout << "Usage: ./sana [OPTION] [ARG(S)] [OPTION] [ARG(S)]..." << endl
             << "Try './sana --help' or './sana -h' for more information." << endl;
        exit(0);
    }
    for (int i = 0; i < argc; i++) originalArgv.push_back(argv[i]);

    vector<string> vArg;
    //add first the base values
    vector<string> baseValues;
    for (string line: defaultArguments) { //from defaultArguments.hpp
        for (string s: split(line, ' ')) {
            vArg.push_back(s);
        }
    }
    if (argc >= 2) {
        //next add values from file
        bool addValuesFromFile = FileIO::fileExists(argv[1]);
        if (addValuesFromFile) {
            vector<string> aux = FileIO::fileToWords(argv[1]);
            vArg.insert(vArg.end(), aux.begin(), aux.end());
        }
        //finally, add command line values
        for (int i = addValuesFromFile ? 2 : 1; i < argc; i++) {
            vArg.push_back(string(argv[i]));
        }
    }

    //initializes the argument arrays declared in SupportedArguments.hpp
    SupportedArguments::validateAndAddArguments();

    //default values for missing arguments
    for (string s: stringArgs)       strings[s]       = "";
    for (string s: doubleArgs)       doubles[s]       = 0;
    for (string s: boolArgs)         bools[s]         = false;
    for (string s: doubleVectorArgs) doubleVectors[s] = vector<double> (0);
    for (string s: stringVectorArgs) stringVectors[s] = vector<string> (0);

    bool helpFound = false;
    bool timeFound = false;
    unordered_set<string> helpArgs;
    //check to see if there is a help argument
    for (string arg : vArg) {
        if (arg == "-h" or arg == "--help") helpFound = true;
        else if (helpFound) helpArgs.insert(arg);

        if (arg == "-t") timeFound = true;
    }
    if (helpFound) {
        SupportedArguments::printAllArgumentDescriptions(helpArgs);
        exit(0);
    }
    // Removed time from default arguments, Instead checks here if time provided
    if (not timeFound) {
        throw runtime_error("please specify a run time in minutes using the"
                            " '-t xxx' option; type './sana -h' for help");
    }

    for (uint i = 0; i < vArg.size(); i++) {
        string arg = vArg[i];
        if(strings.count(arg)) {
            strings[arg]=vArg[i+1];
            i++;
        } else if (doubles.count(arg)) {
            doubles[arg]=stod(vArg[i+1]);
            i++;
        } else if (bools.count(arg)) {
            bools[arg] = true;
        } else if (doubleVectors.count(arg)) {
            int k = stoi(vArg[i+1]);
            doubleVectors[arg] = vector<double> (0);
            for (int j = 0; j < k; j++) {
                doubleVectors[arg].push_back(stod(vArg[i+2+j]));
            }
            i = i+k+1;
        } else if (stringVectors.count(arg)){
            int k = stoi(vArg[i+1]);
            stringVectors[arg] = vector<string>(0);
            for (int j = 0; j < k; j++)
                stringVectors[arg].push_back(vArg[i+2+j]);
            i = i+k+1;
        } else {
            if (arg.size() > 1)
                throw runtime_error("Unknown argument: "+arg+". See the README for the correct syntax");
        }
    }
}

void ArgumentParser::writeArguments() {
    cout << "=== Parsed arguments ===" << endl;
    for (auto kv : strings) {
        if (kv.second != "") cout << kv.first << ": " << kv.second << '\t';
    }
    cout << endl;
    for (auto kv : doubles) {
        if (kv.second != 0) cout << kv.first << ": " << kv.second << '\t';
    }
    cout << endl;
    for (auto kv : bools) {
        if (kv.second) cout << kv.first << ": " << kv.second << '\t';
    }
    cout << endl;
    for (auto kv : doubleVectors) {
        if (kv.second.size() != 0) {
            cout << kv.first << ": ";
            for (uint i = 0; i < kv.second.size(); i++) cout << kv.second[i] << " ";
            cout << endl;
        }
    }
    cout << endl;
    for (auto kv : stringVectors) {
        if (kv.second.size() != 0) {
            cout << kv.first << ": ";
            for (uint i = 0; i < kv.second.size(); i++) cout << kv.second[i] << " ";
            cout << endl;
        }
    }
    cout << endl;
}
