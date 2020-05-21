#include "FileIO.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h> //strerror
#include <algorithm>

vector<string> FileIO::fileToLines(const string& fileName) {
    return breakFileIntoStrings(fileName, true);
}

vector<string> FileIO::fileToWords(const string& fileName) {
    return breakFileIntoStrings(fileName, false);
}

vector<string> FileIO::breakFileIntoStrings(const string& fileName, bool asLines) {
    checkFileExists(fileName);
    char buf[10240];
    bool isPiped;
    FILE *fp = readFileAsFilePointer(fileName, isPiped);
    vector<string> result;
    if (asLines) {
        while (fgets(buf, sizeof(buf), fp)) {
            string line(buf);
            if (not line.empty() and line.back() == '\r') line.pop_back();
            result.push_back(line);
        }
    }
    else { //as words
        while (fscanf(fp, "%s", buf)>0) {
            string word(buf);
            if (not word.empty() and word.back() == '\r') word.pop_back();
            result.push_back(word);
        }
    }
    closeFile(fp, isPiped);
    return result;
}

vector<vector<string>> FileIO::fileToWordsByLines(const string& fileName) {
    checkFileExists(fileName);
    ifstream ifs(fileName);
    vector<vector<string>> result(0);
    string line;
    while (safeGetLine(ifs, line)) {
        istringstream iss(line);
        vector<string> words;
        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(words));
        result.push_back(words);
    }
    return result;
}

istream& FileIO::safeGetLine(istream& is, string& line) {
	istream& res = getline(is, line);
	if (not line.empty() and line.back() == '\r') line.pop_back();
	return res;
}

bool FileIO::fileExists(const string& fileName) {
    if (FILE *file = fopen(fileName.c_str(), "r")) {
        fclose(file);
        return true;
    }
    return false;
}

void FileIO::checkFileExists(const string& fileName) {
    if (not fileExists(fileName)) throw runtime_error("File "+fileName+" not found");
}

string FileIO::addUniquePostfixToFileName(const string& fileName, const string& extension) {
    string res = fileName;
    int i = 2;
    while (fileExists(res + extension)) {
        res = fileName + "_" + to_string(i);
        i++;
    }
    return res + extension;
}

void FileIO::closeFile(FILE* fp, const bool& isPiped) {
    if (isPiped) pclose(fp);
    else fclose(fp);
}

stdiobuf FileIO::readFileAsStreamBuffer(const string& fileName) {
    bool isPiped;
    FILE* f = readFileAsFilePointer(fileName, isPiped);
    return stdiobuf(f, isPiped);
}

FILE* FileIO::readFileAsFilePointer(const string& fileName, bool& isPiped) {
    FILE* fp;
    string decompressionProg = getDecompressionProgram(fileName);
    isPiped = false;
    if (decompressionProg != "") {
        fp = decompressFile(decompressionProg, fileName);
        isPiped = true;
    }
    else fp = fopen(fileName.c_str(), "r");
    return fp;
}

string FileIO::getDecompressionProgram(const string& fileName) {
    string ext = fileName.substr(fileName.find_last_of(".") + 1);
    if(ext == "gz") return "gunzip";
    if(ext == "xz") return "xzcat";
    if(ext == "bz2") return "bzip2 -dk";
    return "";
}

string FileIO::getUncompressedFileExtension(const string& fileName) {
    if(getDecompressionProgram(fileName) != "") {
        string noCompressionExt = extractFileNameNoExtension(fileName);
        return noCompressionExt.substr(noCompressionExt.find_last_of(".") + 1);
    }
    return "";
}

FILE* FileIO::decompressFile(const string& decompProg, const string& fileName) {
    cerr << "decompressFile: decompressing using " << decompProg << ": " << fileName << endl;
    string command = decompProg+" < "+fileName; // eg "gunzip < filename.gz"
    return popen(command.c_str(), "r");
}

bool FileIO::folderExists(const string& folderName) {
    DIR* dir = opendir(folderName.c_str());
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

void FileIO::createFolder(const string& folderName) {
    if (not folderExists(folderName)) {
        int res = mkdir(folderName.c_str(), ACCESSPERMS);
        //race condition: if somebody else created it in the meantime (EEXIST), it is OK.
        if (res == -1 && errno != EEXIST)
            throw runtime_error("error creating directory " + folderName
                                + " (errno "+to_string(errno)+", "+strerror(errno)+")");
    }
}

void FileIO::deleteFile(const string& fileName) {
    remove(fileName.c_str());
}

void FileIO::writeDataToFile(const vector<vector<string>>& wordsByRows, const string& fileName, bool useTabs) {
    ofstream ofs(fileName);
    for (uint i = 0; i < wordsByRows.size(); i++) {
        for (uint j = 0; j < wordsByRows[i].size(); j++) {
            ofs << wordsByRows[i][j];
            if (j != wordsByRows[i].size() - 1) {
                if (useTabs) ofs << "\t";
                else ofs << " ";
            }
        }
        ofs << endl;
    }
}

uint FileIO::numLinesInFile(const string& fileName) {
    ifstream ifs(fileName); 
    return 1 + count(istreambuf_iterator<char>(ifs), 
                     istreambuf_iterator<char>(), '\n');
}

string FileIO::extractFileName(const string& s) {
    int pos = s.size()-1;
    while (pos >= 0 and s[pos] != '/') pos--;
    return s.substr(pos+1);
}

string FileIO::extractFileNameNoExtension(const string& s) {
    string res = extractFileName(s);
    //remove suffix starting with '.'
    int pos = res.size() - 1;
    while (pos >= 0 and res[pos] != '.') pos--; //gives position of last "."
    return res.substr(0, pos);
}

void FileIO::skipWordInStream(istream& is, const string& word) {
    string s;
    if (is >> s) {
        if (s.back() == '\r') s.pop_back();
        if (s != word) throw runtime_error("expected word '"+word+"'' but got '"+s+"'");
    } else {
        throw runtime_error("expected word '"+word+"' but couldn't read from stream");
    }
}

bool FileIO::canSkipWordInStream(istream& is, const string& str) {
    string s;
    if (is >> s) {
        if (s.back() == '\r') s.pop_back();
        return str == s;
    }
    return false;
}
