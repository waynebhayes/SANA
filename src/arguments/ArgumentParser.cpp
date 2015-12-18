#include <iostream>
#include "ArgumentParser.hpp"
#include "../utils.hpp"
using namespace std;

ArgumentParser::ArgumentParser(
    const vector<string>& defStringArgs,
    const vector<string>& defDoubleArgs,
    const vector<string>& defBoolArgs,
    const vector<string>& defVectorArgs) {
  initDefaultValues(defStringArgs, defDoubleArgs, defBoolArgs, defVectorArgs);

}

void ArgumentParser::initDefaultValues(
    const vector<string>& defStringArgs,
    const vector<string>& defDoubleArgs,
    const vector<string>& defBoolArgs,
    const vector<string>& defVectorArgs) {

  for (string s : defStringArgs) {
    strings[s] = "";
  }
  for (string s : defDoubleArgs) {
    doubles[s] = 0;
  }
  for (string s : defBoolArgs) {
    bools[s] = false;
  }
  for (string s : defVectorArgs) {
    vectors[s] = vector<double> (0);
  }
}

void ArgumentParser::parseArgs(int argc, char* argv[],
  vector<string> baseValueLines, bool readParamsFromFileInFirstArg) {

  originalArgv = vector<string> (argc);
  for (int i = 0; i < argc; i++) {
    originalArgv[i] = argv[i];
  }

  vector<string> baseValues = vector<string> ();
  for (string line : baseValueLines) {
    for (string s : split(line, ' ')) {
      baseValues.push_back(s);
    }
  } 

  vector<string> allArgs = getFullArgList(originalArgv, baseValues,
    readParamsFromFileInFirstArg);

  initParsedValues(allArgs);

}

vector<string> ArgumentParser::getFullArgList(const vector<string>& argv,
  const vector<string>& baseValues, bool readParamsFromFileInFirstArg) {

  uint argc = argv.size();

  vector<string> result(0);

  //add first the base values
  for (string s : baseValues) {
    result.push_back(s);
  }
  if (argc < 2) return result;

  //next add values from file
  bool addValuesFromFile = readParamsFromFileInFirstArg and fileExists(argv[1]);
  if (addValuesFromFile) {
    vector<string> aux = fileToStrings(argv[1]);
    result.insert(result.end(), aux.begin(), aux.end());
  }

  //finally add command line values
  for (uint i = addValuesFromFile ? 2 : 1; i < argc; i++) {
    result.push_back(string(argv[i]));
  }

  return result;
}

void ArgumentParser::initParsedValues(vector<string> vArg) {
  int n = vArg.size();
  //check to see if there is a help argument
  for (string arg : vArg) {
    if (strEq(arg, "-h") or strEq(arg, "--help")) {
      cerr << "See the README to see the available arguments." << endl;
      exit(0);
    }
  }

  string arg = "";
  int i = 0;
  while (i < n) {
    arg = vArg[i];
    if(strings.count(arg)) {
      strings[arg]=vArg[i+1];
      i++;
    }
    else if(doubles.count(arg)) {
      doubles[arg]=stod(vArg[i+1]);
      i++;
    }
    else if(bools.count(arg)) bools[arg] = true;
    else if(vectors.count(arg)) {
      int k = stoi(vArg[i+1]);
      vectors[arg] = vector<double> (0);
      for (int j = 0; j < k; j++) {
        vectors[arg].push_back(stod(vArg[i+2+j]));
      }
      i = i+2+k-1;
    }
    else {
    	throw runtime_error("Unknown argument: " + arg + ". See the README for the correct syntax");
    }
    ++i;
  }
}

void ArgumentParser::writeArguments() {
  cerr << "=== Parsed arguments ===" << endl;
  for (auto it = strings.cbegin(); it != strings.cend(); ++it)
    cerr << (*it).first << ": " << (*it).second << '\t';
  cerr << endl;
  for (auto it = doubles.cbegin(); it != doubles.cend(); ++it)
    cerr << (*it).first << ": " << (*it).second << '\t';
  cerr << endl;
  for (auto it = bools.cbegin(); it != bools.cend(); ++it)
    cerr << (*it).first << ": " << (*it).second << '\t';
  cerr << endl;
  for (auto it = vectors.cbegin(); it != vectors.cend(); ++it) {
    cerr << (*it).first << ": ";
    for (uint i = 0; i < (*it).second.size(); i++)
      cerr << (*it).second[i] << " ";
    cerr << endl;
  }
}
