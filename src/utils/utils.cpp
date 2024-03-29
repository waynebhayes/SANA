#include "utils.hpp"
#include <random>
#include <vector>
#include <ctime>
#include <map>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <set>
#include <algorithm>
#include "randomSeed.hpp"

using namespace std;

const string AUTOGENEREATED_FILES_FOLDER = "autogenerated/"; 

// All comparisons with nan variables are false, including self-equality.
// Thus if it's not equal to itself, it's NAN.
// Note you can't just do (x!=x), because that's always false, NAN or not.
bool myNan(double x) { return !(x==x); }

// FIXME: this results in ONE GLOBAL generator for ALL threads that call randInt; same for randDouble
// Thus, *even though the functions are thread-safe*, there is randomness inherent in the OS-level context switching,
// and so different threads will receive an unpredicable set of pulls from the single constant stream associated with a
// fixed seed. To fix... you need to pass the gen in to ALL these functions, down from SANA's personal copy.
double randDouble() {
    static mt19937 gen(getRandomSeed());
    static uniform_real_distribution<> realDis(0, 1);
    return realDis(gen);
}

int randInt(int low, int high) {
    static mt19937 gen(getRandomSeed());
    uniform_int_distribution<> dis(low, high);
    return dis(gen);
}

int randMod(int n) { return randInt(0, n-1); }

void randomShuffle(vector<uint>& v) {
    random_shuffle(v.begin(), v.end(), randMod);
}

double vectorMean(const vector<double>& v) {
    double sum = 0;
    for (double d : v) sum += d;
    return sum/v.size();
}

void printTable(const vector<vector<string>>& table, int colSeparation, ostream& stream) {
    int rows = table.size();
    int cols = table[0].size();
    vector<uint> colWidths(cols, 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            colWidths[j] = max(colWidths[j], (uint) table[i][j].size());
        }
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            stream << table[i][j];
            int tab = colWidths[j] - table[i][j].size() + colSeparation;
            for (int k = 0; k < tab; k++) {
                stream << " ";
            }
        }
        stream << endl;
    }
}

string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

void normalizeWeights(vector<double>& weights) {
    double sum = 0;
    uint n = weights.size();
    for (uint i = 0; i < n; i++) sum += weights[i];
    if (sum > 0) {
        for (uint i = 0; i < n; i++) weights[i] = weights[i]/sum;
    }
}

string extractDecimals(double value, int count) {
    string valueString = to_string(value);
    int k = 0;
    int n = valueString.size();
    while(k < n and valueString[k] != '.') k++;
    string result = "";
    k++;
    int pos = k;
    while (k < min(pos+count, n)) {
        result += valueString[k];
        k++;
    }
    while (result.size() < (uint) count) result += "0";
    return result;
}

string exec(string cmd) {
    //cout << "exec(" + cmd + ");" << endl;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw "Error executing " + cmd + "\n";
    char buffer[128];
    string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL) result += buffer;
    }
    pclose(pipe);
    return result;
}

string execWithoutPrintingErr(string cmd) {
    cmd += " 2>/dev/null";
    return exec(cmd);
}

void execPrintOutput(string cmd) {
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

string toLowerCase(const string& s) {
    string res = s;
    for (uint i = 0; i < s.size(); i++)
        if (s[i] >= 'A' and s[i] <= 'Z')
            res[i] = s[i] - 'A' + 'a';
    return res;
}

vector<string> nonEmptySplit(const string& s, char c) {
    vector<string> res;
    string curr = "";
    for (char x : s) {
        if (x == c) {
            if (curr != "") {
                res.push_back(curr);
                curr = "";
            }
        } else {
            curr.push_back(x);
        }
    }
    if (curr != "") res.push_back(curr);
    return res;
}
