#include <climits>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
using std::vector;
using std::string;
using std::stringstream;
using std::cout;

string appendLeadingZeros(const string& inputBitstring, int numNodes) {
	size_t bitStringFinalSize = static_cast<size_t>(numNodes * (numNodes - 1) / 2);
	size_t inputSize = inputBitstring.size();
	if (bitStringFinalSize > inputSize) {
		size_t leadingZeros = bitStringFinalSize - inputSize;
		stringstream ss;
		for (size_t i = 0; i < leadingZeros; i++) {
			ss << '0';
		}
		ss << inputBitstring;
		return ss.str();
	}
	return inputBitstring;
}

string toBitString(unsigned long long inputDecimalNum, int numNodes) {
    stringstream ss;

	//Convert input decimal to reversed bitstring
	while (inputDecimalNum) {
		ss << (inputDecimalNum & 1);
		inputDecimalNum /= 2;
	}

	//Reverse string to correct reverse order of bits
	string result = ss.str();
	std::reverse(result.begin(), result.end());
	return result;
}

int main() {

    for (int k = 3; k <= 3; k++) {
        unsigned long long limit = pow(2, k*(k-1)/2);
        for (unsigned long long i = 0; i < limit; i++) {
            stringstream ss;
            ss << "./gv -k " << k << " -d " << i << " -u -o " << k << "test" << i << " -t " << appendLeadingZeros(toBitString(i, k), k);
            cout << ss.str() << "\n";            
            system(ss.str().c_str());
        }
    }
    vector<int> four = {0, 1, 3, 7, 11, 12, 13, 15, 30, 31, 63};
    vector<int> five = {0, 1, 3, 7, 11, 12, 13, 15, 30, 31, 63, 75, 76, 77, 79, 86, 87, 94, 95, 116, 117, 119, 127, 222, 223, 235, 236, 237, 239, 254, 255, 507, 511, 1023 };
    for (int i : four) {
        stringstream ss;
        ss << "./gv -k " << 4 << " -d " << i << " -u -o " << 4 << "test" << i << " -t " << appendLeadingZeros(toBitString(i, 4), 4);
        cout << ss.str() << "\n";            
        system(ss.str().c_str());
    }
    for (int i : five) {
        stringstream ss;
        ss << "./gv -k " << 5 << " -d " << i << " -u -o " << 5 << "test" << i << " -t " << appendLeadingZeros(toBitString(i, 5), 5);
        cout << ss.str() << "\n";            
        system(ss.str().c_str());
    }
    return 0;
}