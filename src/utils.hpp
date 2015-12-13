#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <map>
#include <ostream>
using namespace std;

#include "templateUtils.cpp"

typedef uint16_t ushort;
typedef uint32_t uint;

double randDouble();
int randInt(int low, int high);
int randMod(int n);

vector<ushort> reverseMapping(const vector<ushort>& map, int range);

int min(int a, int b);
int max(int a, int b);
bool strEq(const string& s1, const string& s2);

double vectorMean(const vector<double>& v);
double vectorMax(const vector<double>& v);
double vectorMin(const vector<double>& v);
double vectorSum(const vector<double>& v);

void randomShuffle(vector<ushort>& v);

void printTable(const vector<vector<string> >& table, int colSeparation, ostream& stream);

vector<string> loadNodeNamesGwFile(const string& fileName);
void loadLocalSimilaritiesListFormat(map<string,ushort>& yeastNamesToIndex, map<string,ushort>& humanNamesToIndex, const string& fileName, vector<vector<float> >& sims);
void loadLocalSimilaritiesMatrixFormat(const string& fileName, vector<vector<float> >& sims, uint n1, uint n2);
map<string,ushort> loadNameToIndexMapping(const string& fileName);

bool fileExists(const string& filename);
void checkFileExists(const string& filename);
void addUniquePostfixToFilename(string& name, const string& extension);

const string currentDateTime();

double alignmentSimilarity(const vector<ushort>& A1, const vector<ushort>& A2);

void transformLocalSimilarityFileFormat(const string& oldFileName, const string& newFileName, const string& G1File, const string& G2File);

void loadDistanceMatrixFile(const string& fileName, vector<vector<short> >& dists, uint n);

string removeDirectoryFromFileName(const string& fileName);
string substrBefore(const string& s, const string& chars);

void normalizeWeights(vector<double>& weights);

vector<string> fileToStrings(const string& fileName, bool asLines = false);
vector<vector<string> > fileToStringsByLines(const string& fileName);

void error(const string& message);

string extractDecimals(double value, int count);

void simFileToBinaryFormat(const string& simFile, const string& binFile, const string& G1File, const string& G2File);

string toString(int n);

bool folderExists(string folderName);
void createFolder(string folderName);

string exec(string cmd);
void execPrintOutput(string cmd);

void deleteFile(string name);

void writeDataToFile(const vector<vector<string> >& data, string fileName, bool useTabs = false);

extern bool interrupt;

void sigIntHandler(int s);

uint factorial(uint n);
uint binomialCoefficient(uint n, uint k);
double binomialCoefficientFloat(uint n, uint k);

vector<string> getFilesInDirectory(const string &directory);
string autocompleteFileName(const string dir, const string fileNamePrefix);

string toLowerCase(string s);

string extractFileName(string s);
string extractFileNameNoExtension(string s);

vector<string> removeDuplicates(const vector<string>& v);

vector<string> split(const string& s, char c);
#endif
