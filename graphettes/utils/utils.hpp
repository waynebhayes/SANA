#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <map>
#include <ostream>
using namespace std;

/*
Auxiliary functions oblivious to SANA or network alignment but that can come helpful.
*/

#include "templateUtils.cpp"

typedef uint16_t ushort;
typedef uint32_t uint;

double randDouble();
int randInt(int low, int high);
int randMod(int n);

vector<ushort> reverseMapping(const vector<ushort>& map, int range);

int min(int a, int b);
int max(int a, int b);

double vectorMean(const vector<double>& v);
double vectorMax(const vector<double>& v);
double vectorMin(const vector<double>& v);
double vectorSum(const vector<double>& v);

void randomShuffle(vector<ushort>& v);
void randomShuffle(vector<vector<string>>& v);

void printTable(const vector<vector<string> >& table, int colSeparation, ostream& stream);

const string currentDateTime();

void normalizeWeights(vector<double>& weights);

vector<string> fileToStrings(const string& fileName, bool asLines = false);
vector<vector<string> > fileToStringsByLines(const string& fileName);

string extractDecimals(double value, int count);

string intToString(int n);

bool fileExists(const string& filename);
void checkFileExists(const string& filename);
void addUniquePostfixToFilename(string& name, const string& extension);

bool folderExists(string folderName);
void createFolder(string folderName);

string exec(string cmd);
string execWithoutPrintingErr(string cmd);
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
