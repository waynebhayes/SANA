#include "WrappedMethod.hpp"

WrappedMethod::WrappedMethod(Graph* G1, Graph* G2, string name, string args): Method(G1, G2, name) {
	string g1Name = G1->getName();
	string g2Name = G2->getName();

	//rand int used to avoid collision between parallel runs
    //these files cannot be moved to the tmp/ folder
    g1FileName = name + "tmp1_" + g1Name + "_" + g2Name + "_" + intToString(randInt(0, 999999));
    g2FileName = name + "tmp2_" + g1Name + "_" + g2Name + "_" + intToString(randInt(0, 999999));

    parameters = args;
}

void WrappedMethod::moveFilesToWrappedDir() {
    exec("mv " + g1File + " " + wrappedDir);
    exec("mv " + g2File + " " + wrappedDir);
}

Alignment WrappedMethod::run() {
	if(parameters == "") {
		loadDefaultParameters();
	}

	g1File = convertAndSaveGraph(G1, g1FileName);
	g2File = convertAndSaveGraph(G2, g2FileName);

	moveFilesToWrappedDir();

    alignmentFile = generateAlignment();

    Alignment A = loadAlignment(G1, G2, alignmentFile);

    deleteAuxFiles();
    return A;
}

void WrappedMethod::describeParameters(ostream& stream) {
	stream << parameters;
}

string WrappedMethod::fileNameSuffix(const Alignment& A) {
    return "";
}


