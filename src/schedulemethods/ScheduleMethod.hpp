#ifndef SCHEDULEMETHOD_HPP
#define SCHEDULEMETHOD_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>

#include "../methods/SANA.hpp"

using namespace std;

class ScheduleMethod {

public:

    ScheduleMethod(SANA *const sana);
    ~ScheduleMethod() =default;

    virtual string getName() =0;

    static constexpr double DEFAULT_TARGET_INITIAL_PBAD = 0.985; 
    static constexpr double DEFAULT_TARGET_FINAL_PBAD = 1e-10; 
    void setTargetInitialPBad(double pBad);
    void setTargetFinalPBad(double pBad);

    double getTInitial();
    double getTFinal();

    //alternative to computing TIni and TFin
    //for methods that can compute temps for any pBad and not just TIni and TFin
    //not all methods need implement this (defaults to throwing error)
    virtual double computeTempForPBad(double pBad);

    void printScheduleStatistics();
    vector<double> dataForComparison();
protected:
    SANA *const sana; //used mainly for getPBad

    double targetInitialPBad, targetFinalPBad;
    double TInitial, TFinal;
    double TInitialTime, TFinalTime;
    int TInitialSamples, TFinalSamples;

    virtual void computeTInitial() =0;
    virtual void computeTFinal() =0;

    //auxiliary functions used by several schedule methods:

    double getPBad(double temp, double maxTime = 1.0, int logLevel = 1);
    multimap<double, double> tempToPBad; //every call to getPBad adds an entry to this map
    static multimap<double, double> allTempToPBad; //and here, the union of all the methods

    double doublingMethod(double targetPBad, bool nextAbove, double base = 10, double getPBadTime = 1);

    // Binary search based on pbads
    double pBadBinarySearch(double pBad);

    //uses the tempToPBad map to do regression based on all the pbads collected by this method
    void setTInitialTFinalFromRegression(bool useDataFromAllMethods = false);

    void populatePBadCurve();

    bool hasComputedTInitial;
    bool hasComputedTFinal;

private:

/*

    double execTime;


    Alignment runAndPrintTime();
    virtual Alignment run() =0;
    virtual void describeParameters(ostream& stream) =0;
    virtual string fileNameSuffix(const Alignment& A) =0;


*/
};

#endif


