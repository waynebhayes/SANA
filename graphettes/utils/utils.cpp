#include <random>
#include <vector>
#include <ctime>
#include <map>
#include <unordered_set>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <set>
#include <string.h>
#include <iterator>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "utils.hpp"
#include "randomSeed.hpp"

using namespace std;

mt19937 gen(getRandomSeed());
ranlux24_base fastGen(getRandomSeed());
uniform_real_distribution<> realDis(0, 1);

double randDouble() {
    return realDis(fastGen);
}

int randInt(int low, int high) {
    uniform_int_distribution<> dis(low, high);
    return dis(fastGen);
}

int randMod(int n) {
    return randInt(0, n-1);
}

double vectorMean(const vector<double>& v) {
    double sum = 0;
    for (double d : v) sum += d;
    return sum/v.size();
}

double vectorMax(const vector<double>& v) {
    double m = v[0];
    for (uint i = 1; i < v.size(); i++) m = max(m, v[i]);
    return m;
}

double vectorMin(const vector<double>& v) {
    double m = v[0];
    for (uint i = 1; i < v.size(); i++) m = min(m, v[i]);
    return m;
}

double vectorSum(const vector<double>& v) {
    double m = 0;
    for (uint i = 0; i < v.size(); i++) m += v[i];
    return m;
}

void randomShuffle(vector<ushort>& v) {
    random_shuffle(v.begin(), v.end(), randMod);
}

void randomShuffle(vector<vector<string>>& v) {
    random_shuffle(v.begin(), v.end(), randMod);
}

//result[map[i]] = i
vector<ushort> reverseMapping(const vector<ushort>& map, int range) {
    vector<ushort> result(range, -1); //-1 overflows for ushort
    for (uint i = 0; i < map.size(); i++) {
        result[map[i]] = i;
    }
    return result;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

void printTable(const vector<vector<string> >& table, int colSeparation, ostream& stream) {
    int rows = table.size();
    int cols = table[0].size();
    vector<int> colWidths(cols, 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            colWidths[j] = max(colWidths[j], table[i][j].size());
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

bool fileExists(const string& fileName) {
    if (FILE *file = fopen(fileName.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

void checkFileExists(const string& fileName) {
    stringstream errorMsg;
    if (not fileExists(fileName)) {
        errorMsg << "File " << fileName << " not found" << endl;
        throw runtime_error(errorMsg.str().c_str());
    }
}

void addUniquePostfixToFilename(string& name, const string& fileExtension) {
    string origName = name;
    if (fileExists(name + fileExtension)) {
        name += "_2";
    }
    int i = 3;
    while (fileExists(name + fileExtension)) {
        string n = intToString(i);
        name = origName + "_" + n;
        i++;
    }
}

const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
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

vector<string> fileToStrings(const string& fileName, bool asLines) {
    checkFileExists(fileName);
    ifstream ifs(fileName.c_str());
    vector<string> result(0);
    if(asLines) {
        string line;
        while (getline(ifs, line)) result.push_back(line);
    }
    else {
        string word;
        while (ifs >> word) result.push_back(word);
    }
    ifs.close();
    return result;
}

vector<vector<string> > fileToStringsByLines(const string& fileName) {
checkFileExists(fileName);
    ifstream ifs(fileName.c_str());
    vector<vector<string> > result(0);
    string line;
    while (getline(ifs, line)) {
        istringstream iss(line);
        vector<string> words;
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(words));
        result.push_back(words);
    }
    ifs.close();
    return result;
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

string intToString(int n) {
    return static_cast<ostringstream*>( &(ostringstream() << n) )->str();
}

bool folderExists(string folderName) {
    DIR* dir = opendir(folderName.c_str());
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

void createFolder(string folderName) {
    if (not folderExists(folderName)) {
        int res = mkdir(folderName.c_str(), ACCESSPERMS);
        if (res == -1) throw runtime_error("error creating directory " + folderName + " ("+strerror(errno)+")");
    }
}

string exec(string cmd) {
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

string execWithoutPrintingErr(string cmd) {
    cmd += " 2>/dev/null";
    return exec(cmd);
}

void execPrintOutput(string cmd) {
    cmd += " 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw "Error executing " + cmd;
    char buffer[128];
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            cerr << buffer;
    }
    pclose(pipe);
}

void deleteFile(string name) {
    remove(name.c_str());
}

void writeDataToFile(const vector<vector<string> >& data, string fileName, bool useTabs) {
    ofstream outfile;
    outfile.open(fileName.c_str());
    for (uint i = 0; i < data.size(); i++) {
        for (uint j = 0; j < data[i].size(); j++) {
            outfile << data[i][j];
            if (j != data[i].size() - 1) {
                if (useTabs) outfile << "\t";
                else outfile << " ";
            }
        }
        outfile << endl;
    }
    outfile.close();
}

bool interrupt;
void sigIntHandler(int s) {
    interrupt = true;
    cerr << "Save alignment? (y/n)" << endl << ">> ";
    char c;
    cin >> c;
    if (c != 'y' and c != 'Y') exit(0);
}

uint factorial(uint n) {
    return n>0?n*factorial(n-1):1;
}

uint binomialCoefficient(uint n, uint k) {
    if (k > n) throw runtime_error("n choose k with k > n");
    if (k == 0) return 1;
    if (k > n-k) return binomialCoefficient(n, n-k);
    return n*binomialCoefficient(n-1, k-1)/k;
}

double binomialCoefficientFloat(uint n, uint k) {
    if (k > n) throw runtime_error("n choose k with k > n");
    if (k == 0) return 1;
    if (k > n-k) return binomialCoefficient(n, n-k);
    return n*binomialCoefficient(n-1, k-1)/k;
}

//do not include the trailing '/' when calling it
vector<string> getFilesInDirectory(const string &directory) {
    //source: http://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
    DIR *dir;
    class dirent *ent;
    class stat st;
    vector<string> out(0);
    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL) {
        const string file_name = ent->d_name;
        const string full_file_name = directory + "/" + file_name;
        if (file_name[0] == '.')
            continue;
        if (stat(full_file_name.c_str(), &st) == -1)
            continue;
        const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        if (is_directory)
            continue;
        out.push_back(file_name);
    }
    closedir(dir);
    return out;
}

string autocompleteFileName(const string dir, const string fileNamePrefix) {
    vector<string> files = getFilesInDirectory(dir);
    uint n = fileNamePrefix.size();
    for (uint i = 0; i < files.size(); i++) {
        if (files[i].substr(0, n) == fileNamePrefix) return files[i];
    }
    throw runtime_error("file with prefix "+fileNamePrefix+" in directory "+dir+" not found");
}

string toLowerCase(string s) {
    string res = s;
    for (uint i = 0; i < s.size(); i++)
        if (s[i] >= 'A' and s[i] <= 'Z')
            res[i] = s[i] - 'A' + 'a';
    return res;
}

//strips path
string extractFileName(string s) {
    //remove anything up to the last '/'
    int pos = s.size()-1;
    while (pos >= 0 and s[pos] != '/') pos--;
    s = s.substr(pos+1);
    //remove suffix starting with '.'
    return s;
}

//strips path and file extension
string extractFileNameNoExtension(string s) {
    s = extractFileName(s);
    //remove suffix starting with '.'
    uint pos = 0;
    while (pos < s.size() and s[pos] != '.') pos++;
    s = s.substr(0, pos);
    return s;
}

vector<string> removeDuplicates(const vector<string>& v) {
    set<string> s(v.begin(), v.end());
    vector<string> res(0);
    for (string x : s) res.push_back(x);
    return res;
}

vector<string> split(const string& s, char c) {
    vector<string> res(0);
    string currentWord = "";
    for (char x : s) {
        if (x == c) {
            if (currentWord != "") {
                res.push_back(currentWord);
                currentWord = "";
            }
        } else {
            currentWord.push_back(x);
        }
    }
    if (currentWord != "") res.push_back(currentWord);
    return res;
}
