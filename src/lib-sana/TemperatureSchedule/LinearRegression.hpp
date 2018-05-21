//
// Created by Hudson Hughes on 8/2/16.
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <array>
using namespace std;

class LinearRegression {

public:
    LinearRegression(const unordered_map<double, double> &);
    LinearRegression() {};
    tuple<int, double, double, int, double, double, double, double> Run();
private:
    double IncrementalValues( const int &oldIndex1,
                              const int &oldIndex2,
                              const bool &index1Change,
                              const double &sum,
                              const vector<double> &data);

    array<double, 4> IncrementalValues( const int &oldIndex1,
                              const int &oldIndex2,
                              const bool &index1Change,
                              const array<double, 4> &values,
                              const vector<double> &data);

    array<double, 2> LinearLeastSquares ( const array<double, 4> &values, int n);
    double LeastSquaresError  ( int index1, int index2, double constant, const vector<double> &data);
    double LeastSquaresError  ( int index1, int index2, const array<double, 2> constants, const vector<double> &data);
    array<double, 4> InitialValues      ( const int &index1, const int &index2, const vector<double> &data);
    double InitialSum         ( const int &index1, const int &index2, const vector<double> &data);

    //Member variable
    unordered_map<double, double> chart;
    vector<string> temperatures;
};

