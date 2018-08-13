#include <random>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iostream>
#include "Random.hpp"
#include "Utils.hpp"

using namespace std;

RandomNumberGenerator randObj;
mt19937 gen(randObj.GetSeed());
ranlux24_base fastGen(randObj.GetSeed());

int Utils::RandInt(int low, int high) {
    uniform_int_distribution <> dis(low,high);
    return dis(fastGen);
}

vector < vector<string> > Utils::fileToStringsByLines(const string &fileName) {
    checkFileExists(fileName);
    ifstream ifs(fileName.c_str());
    vector < vector<string> > result(0);
    string token;
    while(getline(ifs,token)) {
       istringstream iss(token);
       vector <string> words;
       copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(words)) ;
        result.push_back(words);
    }
    ifs.close();
    return result;
}

void Utils::checkFileExists(const string &fileName) {
    stringstream errorMsg;
    ifstream fileCheck;
    fileCheck.exceptions(ifstream::failbit | ifstream::badbit);
    try {
        fileCheck.open(fileName);
    } catch (std::ifstream::failure &e) {
        errorMsg << "File " << fileName << " not found/or couldn't open" << endl;
        throw runtime_error(errorMsg.str().c_str());
    } catch (...) {
        errorMsg << "An unknown exception has occurred while opening " << fileName << endl;
        throw runtime_error(errorMsg.str().c_str());
    }
    fileCheck.close();
}


bool Utils::checkFileExistsBool(const string &fileName) {
    if(FILE *file = fopen(fileName.c_str(), "r")) {
        fclose(file);
        return true;
    }
    return false;
}

string Utils::exec(string cmd) {
    cerr << "exec(" + cmd + ");" << endl;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw "Error executing " + cmd;
    char buffer[128];
    string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

void Utils::execPrintOutput(string cmd) {
    cmd += " 2>&1";
    cerr << "exec(" + cmd + ");" << endl;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw "Error executing " + cmd;
    char buffer[128];
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            cerr << buffer;
    }
    pclose(pipe);
}
