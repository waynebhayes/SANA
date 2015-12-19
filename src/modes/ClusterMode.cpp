#include "ClusterMode.hpp"
#include "../utils.hpp"
#include "../arguments/modeSelector.hpp"

const string projectFolder = "/extra/wayne0/preserve/nmamano/networkalignment";

string ClusterMode::getName(void) {
    return "ClusterMode";
}

uint ClusterMode::getOArgValueIndex(const vector<string>& argv) {
  int outFileIndex = -1;
  for (uint i = 0; i < argv.size()-1; i++) {
    if (argv[i] == "-o") {
      outFileIndex = i+1;
    }
  }
  if (outFileIndex == -1) {
    throw runtime_error("Did not find -o argument in the command line");
  }
  return outFileIndex;
}

void ClusterMode::run(ArgumentParser& args) {

  int submitCount = args.doubles["-qcount"];
  string outFile = args.strings["-o"];
  
  //indicates that multiple runs will all overwrite outFile
  bool sameOutFile = submitCount > 1 and outFile != "";

  //call submitToCluster -qcount times
  
  if (not sameOutFile) {
    //easy case, identical submits
    for (int i = 0; i < submitCount; i++) {
      submitToCluster(args.originalArgv);
    }
  } else {
    //otherwise, we append a unique suffix to the value of
    //the -o argument to each submit
    int outFileIndex = getOArgValueIndex(args.originalArgv);
    for (int i = 0; i < submitCount; i++) {
      vector<string> vargs(args.originalArgv);
      vargs[outFileIndex] += "_"+toString(i+1);
      submitToCluster(vargs);
    }    
  }
}

void ClusterMode::submitToCluster(const vector<string>& argv) {
  string scriptName = makeScript(argv);
  exec("chmod +x " + scriptName);
  execPrintOutput("qsub " + scriptName);
}

string ClusterMode::makeScript(const vector<string>& argv) {
  int argc = argv.size();
  string scriptName = "tmp/submit";
  addUniquePostfixToFilename(scriptName, ".sh");
  scriptName += ".sh";

  //mode to be used in the cluster
  string qmode = getQModeArgValue(argv);

  ofstream fout(scriptName.c_str());
  fout << "#!/bin/bash" << endl;
  fout << "cd " << projectFolder << endl;

  //add all the same arguments, but replace the value of -mode for the value of -qmode
  for (int i = 0; i < argc; i++) {
    if (argv[i] != "cluster") {
      fout << argv[i] << " ";
    } else {
      fout << qmode << " ";
    }
  }
  fout << endl;
  return scriptName;
}

string ClusterMode::getQModeArgValue(const vector<string>& argv) {
  int argc = argv.size();
  string qmode;
  bool found = false;
  for (int i = 0; i < argc-1; i++) {
    if (argv[i] == "-qmode") {
      found = true;
      qmode = argv[i+1];
    }
  }
  if (not found) {
    throw runtime_error("mandatory parameter -qmode in cluster mode missing");
  }
  if (not validMode(qmode)) {
    throw runtime_error("invalid -qmode value: " + qmode);
  }
  if (qmode == "cluster") {
    throw runtime_error("-qmode value cannot be cluster");
  }
  return qmode;
}
