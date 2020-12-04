#ifndef METHODSELECTOR_HPP_
#define METHODSELECTOR_HPP_

#include "ArgumentParser.hpp"
#include "../methods/Method.hpp"
#include "../Graph.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/SANA.hpp"

Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M);
Method* initSANA(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M); //This is really inelegant but I can't figure out a better way to do it.
SANA* initSANA(vector<Graph>& GV,Graph& SN, ArgumentParser& args, MeasureCombination& M);//multi mode only
#endif /* METHODSELECTOR_HPP_ */
