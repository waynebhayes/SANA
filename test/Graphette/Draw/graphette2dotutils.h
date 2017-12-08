#include <string>
#include <sstream>

using std::string;
using std::stringstream;

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

//Converts 64 bit decimal input into a bit string
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