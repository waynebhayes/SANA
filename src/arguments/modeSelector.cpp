#include "modeSelector.hpp"

#include "../modes/ParameterEstimation.hpp"
#include "../modes/AlphaEstimation.hpp"
#include "../modes/DebugMode.hpp"
#include "../modes/NormalMode.hpp"
#include "../modes/ClusterMode.hpp"
#include "../modes/Experiment.hpp"
#include "../modes/AnalysisMode.hpp"
#include "../modes/SimilarityMode.hpp"

#include "../utils/utils.hpp"

bool validMode(string name) {
    vector<string> validModes = {
        "cluster", "exp", "param", "alpha", "dbg",
        "normal", "analysis", "similarity",
    };
    for (string s : validModes) {
        if (s == name) return true;
    }
    return false;
}


Mode* selectMode(ArgumentParser& args) {
    Mode* mode;

    string name = args.strings["-mode"];
    //std::cout<<"PRINTING MODE TO CHOOSE FROM"<<name<<"\n"<<endl;
    if (name == "cluster") {
        mode = new ClusterMode();
    } else if (name == "exp") {
        mode = new Experiment();
    } else if (name == "param") {
        mode = new ParameterEstimation();
    } else if (name == "alpha") {
        mode = new AlphaEstimation();
    } else if (name == "dbg") {
        mode = new DebugMode();
    } else if (name == "analysis") {
    	mode = new AnalysisMode();
    } else if (name == "similarity") {
    	mode = new SimilarityMode();
    } else if (name == "normal") {
        mode = new NormalMode();
    } else {
        throw runtime_error("Error: unknown mode: " + name);
    }

    return mode;
}
