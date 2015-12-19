#ifndef REPORT_HPP_
#define REPORT_HPP_

#include <string>

#include "Graph.hpp"
#include "Alignment.hpp"
#include "measures/MeasureCombination.hpp"
#include "methods/Method.hpp"

void saveReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, string reportFile);

void makeReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, ofstream& stream);

#endif /* REPORT_HPP_ */
