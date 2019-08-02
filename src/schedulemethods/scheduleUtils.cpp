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
    if (name == LinearRegressionVintage::name) return make_unique<LinearRegressionVintage>(sana);
    if (name == LinearRegressionModern::name)  return make_unique<LinearRegressionModern>(sana);
    if (name == PBadBinarySearch::name)        return make_unique<PBadBinarySearch>(sana);
    if (name == Ameur::name)                   return make_unique<Ameur>(sana);
    if (name == IteratedAmeur::name)           return make_unique<IteratedAmeur>(sana);
    if (name == StatisticalTest::name)         return make_unique<StatisticalTest>(sana);

    throw runtime_error("schedule method "+name+" not found");
}

void scheduleMethodComparison(SANA *const sana, double targetInitialPBad, double targetFinalPBad) {

    //customizable parameters
    int runsPerMethod = 1;
    vector<string> testedMethodNames = {
         LinearRegressionVintage::name, 
         LinearRegressionModern::name,
         PBadBinarySearch::name,
         Ameur::name,
         IteratedAmeur::name,
         StatisticalTest::name
     };


    vector<vector<string>> data;
    data.push_back({"Method","TInitial","PBad","(relative)","Samples","Time",
        "TFinal","PBad","(relative)","Samples","Time","TotalSamples","TotalTime"});


    // Timer T;
    // T.start();
    for (string methodName : testedMethodNames) {
        auto method = getScheduleMethod(methodName, sana);
        method->setTargetInitialPBad(targetInitialPBad);
        method->setTargetFinalPBad(targetFinalPBad);

        for (int i = 0; i < runsPerMethod; i++) {
            logMethodData(method, data);
        }
    }
    // double totalTime = T.elapsed();

    //apply LR on all the samples taken by all the methods
    // LinearRegressionModern LRM(sana);
    // LRM.setTInitialTFinalFromRegression(true);
    // int totalSamples = ScheduleMethod::allTempToPBad.size();

    // data.push_back(methodDataForComparison("superLR", totalTime, -1, totalSamples, -1, true));
    
    cout << endl << endl;
    cout << "Automatic Temperature Schedule Comparison" << endl;
    cout << "Target Initial PBad: " << targetInitialPBad << endl;
    cout << "Target Final PBad:   " << targetFinalPBad << endl;
    printTable(data, 1, cout);
    cout << endl;
}


void logMethodData(const unique_ptr<ScheduleMethod>& method, vector<vector<string>>& table) {

    method->getTInitial();
    method->getTFinal();
    vector<double> data = method->dataForComparison();
    vector<string> row = {method->getName()};
    vector<int> precision = {6,6,6,0,2,9,9,6,0,2,0,2};
    for (uint i = 0; i < data.size(); i++) {
        row.push_back(toStringWithPrecision(data[i], precision[i]));
    }
    
    if (method->getName() == LinearRegressionVintage::name or method->getName() == LinearRegressionModern::name) {
        row[4] = row[5] = row[9] = row[10] = "NA";
    }

    
    table.push_back(row);

}
