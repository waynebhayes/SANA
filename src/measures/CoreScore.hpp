#ifndef CORESCORE_HPP
#define CORESCORE_HPP

#include "../utils/utils.hpp"
#include "../utils/Matrix.hpp"
//can this be a static constexpr SANA variable? -Nil
#define UNWEIGHTED_CORES

using namespace std;

class CoreScoreData {
    public:

#ifdef UNWEIGHTED_CORES
        Matrix<unsigned long> pegHoleFreq;
        vector<unsigned long> numPegSamples; // number of times this node in g1 was sampled.
#endif
        Matrix<double> weightedPegHoleFreq_pBad; // weighted by pBad
        vector<double> totalWeightedPegWeight_pBad;
        Matrix<double> weightedPegHoleFreq_1mpBad; // weighted by 1-pBad
        vector<double> totalWeightedPegWeight_1mpBad;
        Matrix<double> weightedPegHoleFreq_pwPBad; // weighted by actual(pairwise) pBad
        vector<double> totalWeightedPegWeight_pwPBad;
        Matrix<double> weightedPegHoleFreq_1mpwPBad; // weighted by 1-actual pBad
        vector<double> totalWeightedPegWeight_1mpwPBad;

        void initDataStructures(uint n1, uint n2);

        static double trimCoreScore(Matrix<unsigned long>& Freq, vector<unsigned long>& numPegSamples);
        static double trimCoreScore(Matrix<double>& Freq, vector<double>& numPegSamples);

        void incChangeOp(uint source, uint betterHole, double pBad, double meanPBad);
        void incSwapOp(uint source1, uint source2, uint betterDest1, uint betterDest2, double pBad, double meanPBad);
};

#endif
