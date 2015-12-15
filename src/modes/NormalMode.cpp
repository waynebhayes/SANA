#include <cassert>
#include "NormalMode.hpp"

#include "../utils.hpp"

#include "../arguments/MeasureSelector.hpp"
#include "../arguments/MethodSelector.hpp"
#include "../arguments/GraphLoader.hpp"

#include "../Report.hpp"

void NormalMode::run(ArgumentParser& args) {
	createFolders();
	Graph G1, G2;
	initGraphs(G1, G2, args);

	MeasureCombination M;
	initMeasures(M, G1, G2, args);

	Method* method;
	method = initMethod(G1, G2, args, M);
	Alignment A = method->runAndPrintTime();

	A.printDefinitionErrors(G1,G2);
	assert(A.isCorrectlyDefined(G1, G2) and "Resulting alignment is not correctly defined");

	saveReport(G1, G2, A, M, method, args.strings["-o"]);
}
