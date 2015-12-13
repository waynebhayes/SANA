#include "ClusterMode.hpp"
#include "../utils.hpp"

const string projectFolder = "/extra/wayne0/preserve/nmamano/networkalignment";

string makeScript(const vector<string>& argvs);

void submitToCluster(const vector<string>& argvs) {
  string scriptName = makeScript(argvs);
  exec("chmod +x " + scriptName);
  execPrintOutput("qsub " + scriptName);
}

string makeScript(const vector<string>& argvs) {
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
