#include "ClusterMode.hpp"
#include <iostream>
#include "../utils/utils.hpp"
#include "../arguments/modeSelector.hpp"

using namespace std;


string ClusterMode::getProjectFolder() {
  string projectFolder = "/extra/wayne0/preserve/nmamano/sana/";
  if (not folderExists(projectFolder)) {
    throw runtime_error("Project folder not found: "+projectFolder);
  }
  return projectFolder;
}

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

  scriptFileNameArg = args.strings["-qsubscriptfile"];
  outFileName = args.strings["-qsuboutfile"];
  errFileName = args.strings["-qsuberrfile"];

  int submitCount = args.doubles["-qcount"];
  string alignmentFile = args.strings["-o"];

  //indicates that multiple runs will all overwrite alignmentFile
  bool sameOutFile = submitCount > 1 and alignmentFile != "";

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
      vargs[outFileIndex] += "_"+intToString(i+1);
      submitToCluster(vargs);
    }
  }
}

string ClusterMode::getQsubCommand(const string& scriptFile) {
  string cmd = "qsub";
  if (outFileName != "") cmd += " -o " + outFileName;
  if (errFileName != "") cmd += " -e " + errFileName;
  cmd += " " + scriptFile;
  return cmd;
}

void ClusterMode::submitToCluster(const vector<string>& argv) {
  cerr<<0;
  string scriptFile = makeScript(argv);
  cerr<<1;
  exec("chmod +x " + scriptFile);
  cerr<<2;
  exec(getQsubCommand(scriptFile));
  cerr<<3;
}

string ClusterMode::getScriptFileName() {
  if (scriptFileNameArg != "") {
    return scriptFileNameArg;
  }
  string scriptFile = "tmp/submit";
  addUniquePostfixToFilename(scriptFile, ".sh");
  scriptFile += ".sh";
  return scriptFile;
}

string ClusterMode::makeScript(const vector<string>& argv) {
  string scriptFile = getScriptFileName();

  //mode to be used in the cluster
  string qmode = getQModeArgValue(argv);

  ofstream fout(scriptFile.c_str());
  fout << "#!/bin/bash" << endl;
  fout << "cd " << getProjectFolder() << endl;

  //add all the same arguments, but:
  //replace the value of -mode for the value of -qmode
  //remove cluster-mode specific arguments, for cleanliness
  uint i = 0;
  while (i < argv.size()) {
    if (argv[i] == "-qmode" or argv[i] == "-qsubscriptfile" or
      argv[i] == "-qsuboutfile" or argv[i] == "-qsuberrfile") {
      //skip this and next
      i += 2;
    } else {
      if (argv[i] == "cluster") { //this is the original value of '-mode'
        fout << qmode << " ";
      } else {
        fout << argv[i] << " ";
      }
      ++i;     
    }
  }
  fout << endl;
  return scriptFile;
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
