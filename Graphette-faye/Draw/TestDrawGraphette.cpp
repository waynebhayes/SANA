#include <climits>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "graphette2dotutils.h"
using std::vector;
using std::string;
using std::stringstream;
using std::cout;

int main() {
    //Example permuting both upper and lower. Stringstream in loop is inneficient
    for (int k = 3; k <= 3; k++) {
        unsigned long long limit = pow(2, k*(k-1)/2);
        for (unsigned long long i = 0; i < limit; i++) {
            stringstream ss;
            ss << "./graphette2dot -k " << k << " -d " << i << " -u -o " << k << "testu" << i << " -t " << appendLeadingZeros(toBitString(i, k), k);
            cout << ss.str() << "\n";            
            system(ss.str().c_str());
            stringstream ss2;
            ss2 << "./graphette2dot -k " << k << " -d " << i << " -l -o " << k << "testl" << i << " -t " << appendLeadingZeros(toBitString(i, k), k);
            cout << ss2.str() << "\n";            
            system(ss2.str().c_str());
        }
    }

    //Copy pasted upper canonicals from textfiles in test folder. Should be updated with lower eventually.
    vector<int> four = {0, 1, 3, 7, 11, 12, 13, 15, 30, 31, 63};
    vector<int> five = {0, 1, 3, 7, 11, 12, 13, 15, 30, 31, 63, 75, 76, 77, 79, 86, 87, 94, 95, 116, 117, 119, 127, 222, 223, 235, 236, 237, 239, 254, 255, 507, 511, 1023 };
    
    //Example of how to create dot files for all canonicals. Note -u
    for (int i : four) {
        stringstream ss;
        ss << "./graphette2dot -k " << 4 << " -d " << i << " -u -o " << 4 << "testu" << i << " -t " << appendLeadingZeros(toBitString(i, 4), 4);
        cout << ss.str() << "\n";            
        system(ss.str().c_str());
    }
    for (int i : five) {
        stringstream ss;
        ss << "./graphette2dot -k " << 5 << " -d " << i << " -u -o " << 5 << "testu" << i << " -t " << appendLeadingZeros(toBitString(i, 5), 5);
        cout << ss.str() << "\n";            
        system(ss.str().c_str());
    }
    return 0;
}