#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <map>
#include <ostream>
#include "stdiobuf.hpp"
using namespace std;

/*
Auxiliary functions oblivious to SANA or network alignment but that can come helpful.
*/

#include "templateUtils.cpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/access.hpp"

extern bool scheduleOnly;
extern bool multiPairwiseIteration;

typedef uint16_t ushort;
typedef uint32_t uint;

double randDouble();
int randInt(int low, int high);
int randMod(int n);

vector<uint> reverseMapping(const vector<uint>& map, int range);

int min(int a, int b);
int max(int a, int b);

double vectorMean(const vector<double>& v);
double vectorMax(const vector<double>& v);
double vectorMin(const vector<double>& v);
double vectorSum(const vector<double>& v);

void randomShuffle(vector<uint>& v);
void randomShuffle(vector<vector<string>>& v);

void printTable(const vector<vector<string> >& table, int colSeparation, ostream& stream);

const string currentDateTime();

void normalizeWeights(vector<double>& weights);

FILE* decompressFile(const string& decompProg, const string& fileName);

vector<string> fileToStrings(const string& fileName, bool asLines = false);
vector<vector<string> > fileToStringsByLines(const string& fileName);
void memExactFileParseByLine(vector<vector<string> >& result, const string& fileName);

string extractDecimals(double value, int count);

string intToString(int n);

bool fileExists(const string& filename);
void checkFileExists(const string& filename);
void addUniquePostfixToFilename(string& name, const string& extension);

stdiobuf readFileAsStreamBuffer(const string& fileName);
FILE* readFileAsFilePointer(const string& fileName, bool& piped);
void closeFile(FILE* fp, const bool& isPiped);
string getDecompressionProgram(const string& fileName);
string getUncompressedFileExtension(const string& fileName);

bool folderExists(string folderName);
void createFolder(string folderName);

string exec(string cmd);
string execWithoutPrintingErr(string cmd);
void execPrintOutput(string cmd);

void deleteFile(string name);

void writeDataToFile(const vector<vector<string> >& data, string fileName, bool useTabs = false);

extern bool interrupt;
extern bool saveAlignment;
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

bool newerGraphAvailable(const char* graphDir, const char* binaryDir);
pair<unsigned, unsigned> countVecLens(string& fileName);

unordered_map<uint,uint> getReverseMap(const unordered_map<uint,uint>& reverse);


#endif