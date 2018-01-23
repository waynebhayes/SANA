#ifndef UTILS_HPP
#define UTILS_HPP

/*
   Auxiliary functions oblivious to SANA or network alignment but that can come helpful.
*/

#include <zconf.h>

/*
Static Util Class for all utility functions
*/
class Utils {
public:
    static void checkFileExists(const string &fileName);

    static int RandInt(int low, int high);

    static vector < vector<string> > fileToStringsByLines(const string &fileName);
};
#endif
