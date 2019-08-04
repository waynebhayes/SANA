#ifndef SCHEDULEMETHOD_HPP
#define SCHEDULEMETHOD_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>

#include "../methods/SANA.hpp"
#include "../utils/NormalDistribution.hpp"

using namespace std;

class ScheduleMethod {

public:

    //single, static SANA for all schedule methods
    //this needs to be called before computing temps
    static void setSana(SANA *const sana) { ScheduleMethod::sana = sana; }

    ScheduleMethod();
    ~ScheduleMethod() =default;

    virtual string getName() =0;

    static constexpr double DEFAULT_TARGET_INITIAL_PBAD = 0.985; 
    static constexpr double DEFAULT_TARGET_FINAL_PBAD = 1e-10;
    static constexpr double DEFAULT_ERROR_TOL = 0.01;
    static constexpr double DEFAULT_SAMPLE_TIME = 1;

    virtual void setTargetInitialPBad(double pBad) { targetInitialPBad = pBad; }
    virtual void setTargetFinalPBad(double pBad) { targetFinalPBad = pBad; }
    void setErrorTol(double errorTol) { this->errorTol = errorTol; }
    void setSampleTime(double t) { sampleTime = t; }

    //prints and other info collection
    void printScheduleStatistics();
    static void printTargetRange(double targetPBad, double errorTol);
    double totalTime();
    int totalSamples();

    // subclasses should implement EITHER of:
    //"computeTempForPBad": for classes that can compute temps for arbitrary pbads
    //"vComputeTInitial" and "vComputeTFinal": for classes that can only compute TInitial and TFinal

    //for methods that can compute temps for any pBad and not just TInitial and TFinal
    //not all methods need implement this (defaults to throwing error)
    virtual double computeTempForPBad(double targetPBad, double maxTime, int maxSamples);

    //wrappers around the virtual counterparts that log the runtime and # of samples
    double computeTInitial(double maxTime, int maxSamples);
    double computeTFinal(double maxTime, int maxSamples);
protected:
    //default to calling computeTempForPBad. It should be overriden by methods
    //that cannot compute pBads for arbitrary temps
    virtual void vComputeTInitial(double maxTime, int maxSamples);
    virtual void vComputeTFinal(double maxTime, int maxSamples);

    static SANA* sana;

    double targetInitialPBad, targetFinalPBad;
    double errorTol;
    double TInitial, TFinal;

    //use these to interface with the error tolerance rather than using errorTol directly:
    static double targetRangeMin(double targetPBad, double errorTol);
    static double targetRangeMax(double targetPBad, double errorTol);
    static double distToTargetRange(double pBad, double targetPBad, double errorTol);
    static bool isWithinTargetRange(double pBad, double targetPBad, double errorTol);
    static bool isBelowTargetRange(double pBad, double targetPBad, double errorTol);
    static bool isAboveTargetRange(double pBad, double targetPBad, double errorTol);

    //auxiliary functions used by several schedule methods:

    //wrapper around sana->getPBad that saves the result in tempToPBad
    double getPBad(double temp);

    double sampleTime; //time getPBad is allowed to run
    multimap<double, double> tempToPBad; //every call to getPBad adds an entry to this map

    double doublingMethod(double targetPBad, bool nextAbove, double base = 10);

    // Binary search based on pbads
    double pBadBinarySearch(double targetPBad, double maxTime, int maxSamples);

    double tempWithClosestPBad(double targetPBad, double atLeast = -1, double atMost = -1) const; 
    double tempWithBestLRFit(double targetPBad) const; 

    //samples evenly (in log scale) along the pBad spectrum from really low to really high temps
    void populatePBadCurve();

private:

    double TInitialTime, TFinalTime;
    int TInitialSamples, TFinalSamples;


    static double tempWithClosestPBad(double targetPBad, const multimap<double,double>& tempToPBad,
                double atLeast = -1, double atMost = -1);
    static double tempWithBestLRFit(double targetPBad, const multimap<double,double>& tempToPBad);

    //stuff for comparison for paper:
    friend void scheduleMethodComparison(SANA *const sana);
    friend vector<string> formatMethodData(string, double, double, double, double, double, int,
                                double, int, double, int, double, int, double);
    friend NormalDistribution getPBadDis(double temp, int numSamples, double sampleTime);

    //union of the tempToPBad maps of all the methods
    static multimap<double, double> allTempToPBad; 
    static double sGetPBad(double temp, double sampleTime);

};

#endif


