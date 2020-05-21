#ifndef FILEIO_HPP_
#define FILEIO_HPP_

#include <vector>
#include <string>
#include <istream>
#include <cstdio>
#include "stdiobuf.hpp"

using namespace std;

/* class with only static methods
functions in this class should handle both \n and \r\n endlines

Guidelines for reading a file:

If a file is small or read infrequently, the best option is to use one of the functions
that put the entire file directly in a vector, in the most convenient form:
fileToLines / fileToWords / fileToWordsByLines

To read a file line by line for efficiency reasons, you can do:

ifstream ifs(fileName);
FileIO::checkFileExists(fileName);
string line;
while(FileIO::safeGetLine(ifs, line)) {	... }
//ifs is closed automatically at the end of the scope

Using an ifstream outside safeGetLine is not recommended
*/
class FileIO {
public:

static vector<string> fileToLines(const string& fileName);
static vector<string> fileToWords(const string& fileName);
static vector<vector<string>> fileToWordsByLines(const string& fileName);
static uint numLinesInFile(const string& fileName); //if the file ends in a newline, it counts as an extra line

static istream& safeGetLine(istream& is, string& line); //just like getline but handles both "\n" and "\r\n"

static bool fileExists(const string& filename);
static bool folderExists(const string& folderName);
static void checkFileExists(const string& fileName);
static void createFolder(const string& folderName);
static void deleteFile(const string& fileName);

static string extractFileNameNoExtension(const string& s); //strips path and file extension
static string addUniquePostfixToFileName(const string& fileName, const string& extension);

static string getUncompressedFileExtension(const string& fileName);

static stdiobuf readFileAsStreamBuffer(const string& fileName); //handles compressed files
static FILE* readFileAsFilePointer(const string& fileName, bool& isPiped);
static void closeFile(FILE* fp, const bool& isPiped);

static void skipWordInStream(istream& is, const string& str); //can handle \r\n endlines
static bool canSkipWordInStream(istream& is, const string& str); //can handle \r\n endlines

//output functions:
static void writeDataToFile(const vector<vector<string>>& wordsByRows, const string& fileName, bool useTabs = false);

private:

static vector<string> breakFileIntoStrings(const string& fileName, bool asLines);

static string extractFileName(const string& s); //strips path (anything up to the last '/')

static FILE* decompressFile(const string& decompProg, const string& fileName);
static string getDecompressionProgram(const string& fileName);

};

#endif /* FILEIO_HPP_ */

