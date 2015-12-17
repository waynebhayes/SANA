#include "ClusterMode.hpp"
#include "../utils.hpp"

const string projectFolder = "/extra/wayne0/preserve/nmamano/networkalignment";

string ClusterMode::getName(void) {
	return "ClusterMode";
}

void ClusterMode::run(ArgumentParser& args) {
	for (int i = 0; i < args.doubles["-qcount"]; i++) {
		vector<string> vargs(args.vArg);
		for (unsigned int i = 0; i < vargs.size(); i++)
			if (i > 0 and not strEq(args.strings["-o"], "")) {
				for (unsigned int j = 0; j < vargs.size(); j++) {
					if (strEq(vargs[j], "-o")) {
						vargs[j+1] += "_" + toString(i+1);
				}
			}
		}
		submitToCluster(vargs);
	}
}

void ClusterMode::submitToCluster(const vector<string>& argvs) {
  string scriptName = makeScript(argvs);
  exec("chmod +x " + scriptName);
  execPrintOutput("qsub " + scriptName);
}

string ClusterMode::makeScript(const vector<string>& argvs) {
  int argc = argvs.size();
  string scriptName = "tmp/submit";
  addUniquePostfixToFilename(scriptName, ".sh");
  scriptName += ".sh";

  ofstream fout(scriptName.c_str());
  fout << "#!/bin/bash" << endl;
  fout << "cd " << projectFolder << endl;
  for (int i = 0; i < argc; i++) {
    if (not strEq(argvs[i], "-qsub")) {
      fout << argvs[i] << " ";
    }
  }
  fout << endl;
  return scriptName;
}
