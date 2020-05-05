#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>
#include "Graph.hpp"
#include "Alignment.hpp"
#include "measures/MeasureCombination.hpp"
#include "methods/Method.hpp"

using namespace std;

class Report {
public:
static void saveReport(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, Method* method, string reportFile);
static void saveReport(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, Method* method, string reportFile, bool multiPairwiseIteration);
static void saveLocalMeasures(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, const Method* method, string& localMeasureFile);

private:
static void makeReport(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, Method* method, ofstream& stream);
static void makeReport(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, Method* method, ofstream& stream, bool multiPairwiseIteration);
static string ensureFileNameExistsAndOpenOutFile(const string& fileType, string outFileName, 
    ofstream& outfile, const string& G1Name, const string& G2Name, const Method* method, 
    const Alignment& A);
static void printStats(const Graph& G, uint numCCsToPrint, ostream& stream);
};

#endif /* REPORT_HPP_ */
