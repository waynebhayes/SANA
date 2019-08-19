#include <vector>
#ifndef UTILS_HPP
#define UTILS_HPP

/*
   Auxiliary functions oblivious to SANA or network alignment but that can come helpful.
*/

/*
Static Util Class for all utility functions
*/
class Utils {
public:
    static void checkFileExists(const string &fileName);
    static bool checkFileExistsBool(const string &fileName);
    static string exec(string cmd);
    static void execPrintOutput(string cmd);


    static int RandInt(int low, int high);

    static vector < vector<string> > fileToStringsByLines(const string &fileName);
};
#endif
