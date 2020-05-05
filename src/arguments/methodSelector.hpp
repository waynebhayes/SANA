#ifndef METHODSELECTOR_HPP_
#define METHODSELECTOR_HPP_

#include "ArgumentParser.hpp"
#include "../methods/Method.hpp"
#include "../Graph.hpp"
#include "../measures/MeasureCombination.hpp"

namespace methodSelector {

Method* initMethod(const Graph& G1, const Graph& G2, ArgumentParser& args, MeasureCombination& M);
Method* initSANA(const Graph& G1, const Graph& G2, ArgumentParser& args, MeasureCombination& M, string startAligName=""); 

} //namespace methodSelector

#endif /* METHODSELECTOR_HPP_ */
