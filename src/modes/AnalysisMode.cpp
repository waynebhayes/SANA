#include <cassert>
#include "AnalysisMode.hpp"

#include "../utils/utils.hpp"

#include "../arguments/measureSelector.hpp"
#include "../arguments/methodSelector.hpp"
#include "../arguments/graphLoader.hpp"

#include "../report.hpp"

Alignment loadAlignment(int format, string file, Graph &G1, Graph &G2) {
	if(format == 0 || file == "") {
    	cerr << "When using analysis mode specify you must specify both -alignFile and -alignFormat." << endl;
    	exit(-1);
	}

	switch (format) {
	case 1:
		return Alignment::sanaOutloadEdgeList(&G1, &G2, file);
	case 2:
		return Alignment::loadEdgeList(&G1, &G2, file);
	case 3:
		return Alignment::loadPartialEdgeList(&G1, &G2, file);
	case 4:
		return Alignment::loadMapping(file);
	default:
		cerr << "Unsupported alignment format. The following are supported" << endl
			 << "1: sana.out format" << endl
			 << "2: edge list format" << endl
			 << "3: partial edge list format" << endl
			 << "4: load mapping (old sana.out 1 line) format" << endl;
		exit(-1);
	}
}

void AnalysisMode::run(ArgumentParser& args) {
    Graph G1, G2;
    initGraphs(G1, G2, args);

    MeasureCombination M;
    initMeasures(M, G1, G2, args);

    Alignment A = loadAlignment(args.doubles["-alignFormat"], args.strings["-alignFile"], G1, G2);

    A.printDefinitionErrors(G1,G2);
    assert(A.isCorrectlyDefined(G1, G2) and "Resulting alignment is not correctly defined");

    saveReport(G1, G2, A, M, NULL, args.strings["-o"]);
}

string AnalysisMode::getName(void) {
    return "AnalysisMode";
}
