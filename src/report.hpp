#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>

#include "Graph.hpp"
#include "Alignment.hpp"
#include "MultiAlignment.hpp"
#include "measures/MeasureCombination.hpp"
#include "methods/Method.hpp"

void makeReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, ofstream& stream);

void saveReport(const vector<Graph>& GV, Graph& SN, const MultiAlignment& A,
  const MeasureCombination& M, Method* method, string reportFile);

void saveReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, string reportFile);


#endif /* REPORT_HPP_ */
