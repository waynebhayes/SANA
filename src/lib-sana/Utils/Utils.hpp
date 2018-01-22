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
    static int RandInt(int low, int high);
};
#endif
