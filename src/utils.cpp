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
using namespace std;

random_device rd;
unsigned int currentSeed = rd();
mt19937 gen(currentSeed);
ranlux24_base fastGen(currentSeed);

uniform_real_distribution<> realDis(0, 1);

void forceSeed(unsigned int newSeed) {
	currentSeed = newSeed;
	gen.seed(currentSeed);
	fastGen.seed(currentSeed);
}

unsigned int getSeed() {
	return currentSeed;
}

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

//result[map[i]] = i
vector<ushort> reverseMapping(const vector<ushort>& map, int range) {
    vector<ushort> result(range, -1);
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

map<string,ushort> loadNameToIndexMapping(const string& fileName) {
    stringstream errorMsg;
    ifstream infile(fileName.c_str());
    string line;
    //ignore header
    for (int i = 0; i < 4; i++) getline(infile, line);
    //read number of nodes
    int n;
    getline(infile, line);
    istringstream iss(line);
    if (!(iss >> n) or n <= 0) {
        errorMsg << "Failed to read node count: " << line;
        throw runtime_error(errorMsg.str().c_str());
    }
   
    map<string,ushort> nameToIndexMapping;
    string node;
    for (int i = 0; i < n; i++) {
        getline(infile, line);
        istringstream iss(line);
        if (!(iss >> node)) {
            errorMsg << "Failed to read node " << i << " of " << n << ": " << line << " (" << node << ")";
            throw runtime_error(errorMsg.str().c_str());
        }
        int nameLength = node.size()-4;
        node = node.substr(2, nameLength); //strip '|{' and '}|'
        nameToIndexMapping[node] = i;
    }
    infile.close();
    return nameToIndexMapping;
}

void loadLocalSimilaritiesListFormat(map<string,ushort>& yeastNamesToIndex, map<string,ushort>& G2NamesToIndex, const string& fileName, vector<vector<float> >& sims) {
    uint n1 = yeastNamesToIndex.size();
    uint n2 = G2NamesToIndex.size();

    checkFileExists(fileName);
    ifstream infile(fileName.c_str());
    sims = vector<vector<float> > (n1, vector<float> (n2, -1.0));
    cerr << "loading " << fileName << "...";
    for (uint i = 0; i < n1*n2; i++) {
        string G2Name, yeastName;
        float sim;
        infile >> G2Name >> yeastName >> sim;
        int G2Index, yeastIndex;
        G2Index = G2NamesToIndex[G2Name];
        yeastIndex = yeastNamesToIndex[yeastName];
        sims[yeastIndex][G2Index] = sim;
    }
    cerr << "done." << endl;
    infile.close();
}

void loadLocalSimilaritiesMatrixFormat(const string& fileName, vector<vector<float> >& sims, uint n1, uint n2) {
    checkFileExists(fileName);
    ifstream infile;
    infile.open(fileName.c_str());
    sims = vector<vector<float> > (n1, vector<float> (n2, -1.0));
    cerr << "loading " << fileName << "...";
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            infile >> sims[i][j];
        }
    }
    cerr << "done." << endl;
    infile.close();
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
        string n = toString(i);
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

bool strEq(const string& s1, const string& s2) {
    return s1.compare(s2) == 0;
}

double alignmentSimilarity(const vector<ushort>& A1, const vector<ushort>& A2) {
    uint sharedNodes = 0;
    for (uint i = 0; i < A1.size(); i++) {
        if (A1[i] == A2[i]) sharedNodes++;
    }
    return sharedNodes/((double) A1.size());
}

//oldFile is in list format, newFile is in matrix format
void transformLocalSimilarityFileFormat(const string& oldFileName, const string& newFileName, const string& G1File, const string& G2File) {
    map<string,ushort> G1NamesToIndex = loadNameToIndexMapping(G1File.c_str());
    map<string,ushort> G2NamesToIndex = loadNameToIndexMapping(G2File.c_str());
    vector<vector<float> > sims;
    loadLocalSimilaritiesListFormat(G1NamesToIndex, G2NamesToIndex, oldFileName, sims);
    ofstream outfile;
    outfile.open(newFileName.c_str());
    int n1 = G1NamesToIndex.size();
    int n2 = G2NamesToIndex.size();
    for(int i = 0; i < n1; i++) {
        for (int j = 0; j < n2; j++) {
            outfile << sims[i][j] << " ";
        }
        outfile << endl;
    }
}

void loadDistanceMatrixFile(const string& fileName, vector<vector<short> >& dists, uint n) {
    checkFileExists(fileName);
    ifstream infile;
    infile.open(fileName.c_str());
    dists = vector<vector<short> > (n, vector<short> (n));
    for (uint i = 0; i < n; i++) {
        for (uint j = 0; j < n; j++) {
            infile >> dists[i][j];
        }
    }
    infile.close();
}

string substrBefore(const string& s, const string& chars) {
    for (uint i = 0; i < s.size(); i++) {
        for (uint j = 0; j < chars.size(); j++) {
            if (s[i]==chars[j]) return s.substr(0, i);
        }
    }
    return s;
}

string removeDirectoryFromFileName(const string& fileName) {
    uint lastDashPos = 0;
    for (uint i = 0; i < fileName.size(); i++) {
        if (fileName[i] == '/') lastDashPos = i;
    }
    return fileName.substr(lastDashPos+1);
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

void error(const string& message) {
  stringstream errorMsg;
  errorMsg << message << endl;
  throw runtime_error(errorMsg.str().c_str());
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

void simFileToBinaryFormat(const string& simFile, const string& binFile, const string& G1File, const string& G2File) {
    map<string,ushort> G1NamesToIndex = loadNameToIndexMapping(G1File.c_str());
    map<string,ushort> G2NamesToIndex = loadNameToIndexMapping(G2File.c_str());
    vector<vector<float> > sims;
    loadLocalSimilaritiesListFormat(G1NamesToIndex, G2NamesToIndex, simFile, sims);
    writeMatrixToBinaryFile(sims, binFile);
}

string toString(int n) {
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
    error("file with prefix "+fileNamePrefix+" in directory "+dir+" not found");
    return "";
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
