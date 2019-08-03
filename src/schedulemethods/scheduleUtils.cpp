#include <vector>
#include <memory>

#include "scheduleUtils.hpp"
#include "ScheduleMethod.hpp"
#include "LinearRegressionVintage.hpp"
#include "LinearRegressionModern.hpp"
#include "PBadBinarySearch.hpp"
#include "Ameur.hpp"
#include "IteratedAmeur.hpp"
#include "StatisticalTest.hpp"
#include "../utils/utils.hpp"

using namespace std;

unique_ptr<ScheduleMethod> getScheduleMethod(string name, SANA* sana) {
    if (name == LinearRegressionVintage::NAME) 
        return unique_ptr<LinearRegressionVintage>(new LinearRegressionVintage(sana));
    if (name == LinearRegressionModern::NAME)  
        return unique_ptr<LinearRegressionModern>(new LinearRegressionModern(sana));
    if (name == PBadBinarySearch::NAME)        
        return unique_ptr<PBadBinarySearch>(new PBadBinarySearch(sana));
    if (name == Ameur::NAME)                   
        return unique_ptr<Ameur>(new Ameur(sana));
    if (name == IteratedAmeur::NAME)           
        return unique_ptr<IteratedAmeur>(new IteratedAmeur(sana));
    if (name == StatisticalTest::NAME)         
        return unique_ptr<StatisticalTest>(new StatisticalTest(sana));

    throw runtime_error("schedule method "+name+" not found");
}

void scheduleMethodComparison(SANA *const sana) {

    //customizable parameters
    vector<string> testedMethodNames = {
         LinearRegressionVintage::NAME, 
         LinearRegressionModern::NAME,
         PBadBinarySearch::NAME,
         Ameur::NAME,
         IteratedAmeur::NAME,
         StatisticalTest::NAME
     };

    double targetInitialPBad = 0.95;
    double targetFinalPBad = 0.00001;
    int runsPerMethod = 1;
    double maxTime = 60;
    int maxSamples = 60;
    double sampleTime = 1; //max time for getPBad to reach equilibrium
    double errorTol = 0.01;
    int numValidationSamples = 1; //set to a high value for final experiment (30?)
            //to find the *real* pBad at the temperatures given by the methods
            //we get an average of this many getPBad samples 
    

    vector<vector<string>> table;
    table.push_back({"Method","TInitial","PBad","(relative)","Samples","Time",
        "TFinal","PBad","(relative)","Samples","Time","TotalSamples","TotalTime"});

    // Timer T;
    // T.start();
    for (string methodName : testedMethodNames) {
        auto method = getScheduleMethod(methodName, sana);
        method->setTargetInitialPBad(targetInitialPBad);
        method->setTargetFinalPBad(targetFinalPBad);
        method->setSampleTime(sampleTime);
        method->setErrorTol(errorTol);

        for (int i = 0; i < runsPerMethod; i++) {
            table.push_back(methodData(method, maxTime, maxSamples, numValidationSamples));
        }
    }
    // double totalTime = T.elapsed();

    //apply LR on all the samples taken by all the methods
    // LinearRegressionModern LRM(sana);
    // LRM.setTInitialTFinalFromRegression(true);
    // int totalSamples = ScheduleMethod::allTempToPBad.size();

    // table.push_back(methodDataForComparison("superLR", totalTime, -1, totalSamples, -1, true));
    
    cout << endl;
    cout << "=================================================" << endl;
    cout << "=================================================" << endl;
    cout << "=================================================" << endl;
    cout << endl;

    cout << "Automatic Temperature Schedule Comparison" << endl;
    cout << "Max time: " << maxTime << " Max samples: " << maxSamples << endl;
    cout << "Sample time: " << sampleTime << endl;

    cout << "Target Initial PBad: " << targetInitialPBad << " range ";
    ScheduleMethod::printTargetRange(targetInitialPBad, errorTol);
    cout << endl << "Target Final PBad:   " << targetFinalPBad << " range ";
    ScheduleMethod::printTargetRange(targetFinalPBad, errorTol);
    cout << endl << endl;

    printTable(table, 1, cout);

    cout << endl;
    cout << "=================================================" << endl;
    cout << "=================================================" << endl;
    cout << "=================================================" << endl;
    cout << endl;
}


vector<string> methodData(const unique_ptr<ScheduleMethod>& method, double maxTime, int maxSamples, double numValidationSamples) {

    bool singleTime = method->getName() == LinearRegressionVintage::NAME or
                      method->getName() == LinearRegressionModern::NAME;

    if (singleTime) {
        method->computeTInitial(maxTime, maxSamples);
        method->computeTFinal(0, 0);
    } else {
        method->computeTInitial(maxTime/2, maxSamples/2);
        method->computeTFinal(maxTime/2, maxSamples/2);
    }

    vector<double> data = method->dataForComparison(numValidationSamples);
    vector<string> row = {method->getName()};
    vector<int> precision = {6,6,6,0,2,9,9,6,0,2,0,2};
    for (uint i = 0; i < data.size(); i++) {
        row.push_back(toStringWithPrecision(data[i], precision[i]));
    }
    
    if (singleTime) {
        row[4] = row[5] = row[9] = row[10] = "NA";
    }

    return row;

}
