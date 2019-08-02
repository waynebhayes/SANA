#include <iostream>
#include <vector>

#include "Ameur.hpp"
#include "../utils/utils.hpp"

using namespace std;

Ameur::Ameur(SANA *const sana) :
    ScheduleMethod(sana) {} 

void Ameur::computeTInitial() {
    TInitial = computeTempForPBad(targetInitialPBad);
}

void Ameur::computeTFinal() {
    TFinal = computeTempForPBad(targetFinalPBad);
}

double Ameur::computeTempForPBad(double pBad) {
    double startTempGuess = 1;
    double unused; //set by reference in call below
    vector<double> EIncs = getEIncSample(startTempGuess, 10000, unused);
    return computeTempForPBad(pBad, startTempGuess, EIncs);
}

//how far from the targetPBad we are allowed to be (in relative terms)
double Ameur::tolerance(double targetPBad) {
    //empirically found values without much experimentation (may not be the best)
    if (targetPBad < 0.1) return 0.2;
    return 0.005;
}

double Ameur::computeTempForPBad(double targetPBad, double startTempGuess, vector<double> EIncs) {
    double errorTolerance = tolerance(targetPBad);

    int maxIters = 100;
    double tempGuess = startTempGuess;
    bool converged = false;
    int iteration = 0;

    double paramP = 1.0; //parameter 'p' in the paper, must be >= 1. higher value is slower but is more likely to converge
    int n = EIncs.size();

    while (not converged and iteration < maxIters) {
        vector<double> pBads(n);
        for (int i = 0; i < n; i++) {
            pBads[i] = max(0.0, min(1.0, exp(-EIncs[i]/tempGuess)));
        }
        double pBadMean = vectorMean(pBads);
        // cout<<"  iteration " << iteration << ": temp: " << tempGuess << " pBad:" << pBadMean << endl;

        converged = pBadMean > targetPBad*(1-errorTolerance) and pBadMean < targetPBad*(1+errorTolerance);
        if (converged) break;

        double nextGuess = tempGuess * pow((log(pBadMean)/log(targetPBad)), 1.0/paramP);

        tempGuess = nextGuess;
        iteration++;
    }
    if (converged) {
        cout << "temp converged to " << tempGuess << " in " << iteration << " iterations" << endl;
    } else {
        cout << "temp reached " << tempGuess << " but did NOT converge after " << iteration << " iterations" << endl;
    }
    return tempGuess;
}

vector<double> Ameur::getEIncSample(double temp, int sampleSize, double& resPBad) {
    //this call fills the pBad buffer from which we get the EInc samples
    //resPBad is passed back by reference because it is needed in Ameur method
    resPBad = getPBad(temp, 2);

    int numPBadsInBuffer = sana->numPBadsInBuffer;

    if (sampleSize > numPBadsInBuffer) {
        cerr << "sample size too large, returning a sample of size " << numPBadsInBuffer << " instead" << endl;
        sampleSize = numPBadsInBuffer;
    }
    vector<double> EIncs(sampleSize);
    for (int i = 0; i < sampleSize; i++) {
        EIncs[i] = -temp * log( (sana->pBadBuffer)[i] );
    }
    return EIncs;
}
