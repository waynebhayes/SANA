#include "Method.hpp"
#include "utils.hpp"
#include "Timer.hpp"
#include <string>

Method::Method(Graph* G1, Graph* G2, string name): G1(G1), G2(G2), name(name) {};

Method::~Method() {
}

string Method::getName() {
	return name;
}

Alignment Method::runAndPrintTime() {
	cerr << "Start execution of " << name << endl;
	Timer T;
	T.start();
	Alignment A = run();
	execTime = T.elapsed();
	cerr << "Executed " << name << " in " << T.elapsedString() << endl;
	return A;
}

double Method::getExecTime() {
	return execTime;
}