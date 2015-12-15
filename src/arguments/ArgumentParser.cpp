#include <iostream>
#include "ArgumentParser.hpp"
#include "../utils.hpp"
using namespace std;

vector<string> getArgumentList(int argc, char* argv[], bool useFileWithDefaultValues,
	string fileWithDefaultValues, bool readParamsFromFileInFirstArg) {

  vector<string> result;
  if (useFileWithDefaultValues) {
  	if (fileExists(fileWithDefaultValues)) {
  		result = fileToStrings(fileWithDefaultValues);
  	}
  	else {
  		throw runtime_error("Cannot find file with default argument values: " + fileWithDefaultValues);
  	}
  }

  if (argc < 2) return result;
  int i = 1;
  if (readParamsFromFileInFirstArg and fileExists(argv[1])) {
    vector<string> aux = fileToStrings(argv[1]);
    result.insert(result.end(), aux.begin(), aux.end());
    i++;
  }

  for (; i < argc; i++) {
    result.push_back(string(argv[i]));
  }

  return result;
}

vector<string> getArgumentList(int argc, char* argv[],
  vector<string> defaultValues, bool readParamsFromFileInFirstArg) {

  vector<string> result;
  for (string arg : defaultValues) {
    for (string s : split(arg, ' ')) {
      result.push_back(s);
    }
  }

  if (argc < 2) return result;
  int i = 1;
  if (readParamsFromFileInFirstArg and fileExists(argv[1])) {
    vector<string> aux = fileToStrings(argv[1]);
    result.insert(result.end(), aux.begin(), aux.end());
    i++;
  }

  for (; i < argc; i++) {
    result.push_back(string(argv[i]));
  }

  return result;
}


ArgumentParser::ArgumentParser(char listStringArgs[][80], char listDoubleArgs[][80], char listBoolArgs[][80], char listVectorArgs[][80]) {

  int i = 0;
  while (not strEq(listStringArgs[i], "")) {
    strings[listStringArgs[i]] = "";
    i++;
  }
  i = 0;
  while (not strEq(listDoubleArgs[i], "")) {
    doubles[listDoubleArgs[i]] = 0;
    i++;
  }
  i = 0;
  while (not strEq(listBoolArgs[i], "")) {
    bools[listBoolArgs[i]] = false;
    i++;
  }
  i = 0;
  while (not strEq(listVectorArgs[i], "")) {
    vectors[listVectorArgs[i]] = vector<double> (0);
    i++;
  }
}

void ArgumentParser::parse(vector<string> vArg) {
  this->vArg = vector<string>(vArg);
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
