#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
//Requires c++11 for stoull
#include <string>
#include <cstring>
#include <cstdio>
#include <math.h>

using std::cout;
using std::cerr;
using std::string;
using std::stringstream;
using std::ofstream;
using std::ifstream;

void parseInput(int argc, char* argv[], int& numNodes, string& inputBitstring, string& outputFile, string& namesFile, bool& isUpper, string& graphTitle);	
void printUsage();
void printHelp();
string toBitString(unsigned long long inputDecimalNum, int numNodes);
string appendLeadingZeros(const string& inputBitstring, int numNodes);

void createDotfileFromBit(int numNodes, const string& inputBitstring, const string& outputFile, const string& namesFile, bool isUpper, const string& graphTitle);
string getPos(int i, int numNodes);
void writeEdges(ofstream& outfile, const string& inputBitstring, int numNodes, bool isUpper);
void writeEdgesUpper(ofstream& outfile, const string& inputBitstring, int numNodes);
void writeEdgesLower(ofstream& outfile, const string& inputBitstring, int numNodes);
void runGraphviz(const string& fileName);

const int RADIUS_SCALING = 25;
const string USAGE = "USAGE: ./gv <-k number_of_nodes> <-b bitstring | -d decimal_representation> <-o output_filename> [-n input_filename] [-u | -l] [-t title] -h for verbose help\n";
const string EDGE_ARGS = "";
const string NODE_ARGS = "";
const string DECIMAL_INPUT_WARNING = "Warning. Decimal input was used with k > 11. Edge information may have been lost.\n";
const double PI  =3.141592653589793238463;

int main(int argc, char* argv[]) {
	int numNodes = 0;
	string inputBitstring = "", outputFile = "", namesFile = "", graphTitle = "";
	bool isUpper = false;

	//Parses input passing variables by reference.
	parseInput(argc, argv, numNodes, inputBitstring, outputFile, namesFile, isUpper, graphTitle);

	createDotfileFromBit(numNodes, inputBitstring, outputFile, namesFile, isUpper, graphTitle);

	runGraphviz(outputFile);

	return EXIT_SUCCESS;
}

/**
 * Parses command line input.
 * Doesn't allow for repeated inputs.
 * Prints usage and exits if invalid input is passed.
 * */
void parseInput(int argc, char* argv[], int& numNodes, string& inputBitstring, string& outputFile, string& namesFile, bool& isUpper, string& graphTitle) {
	bool k = false, inputType = false, o = false, n = false, matrixType = false, title = false;
	unsigned long long inputDecimalNum = 0;	
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-k") == 0) {
			if (k) {
				cerr << "Only one k is allowed\n";
				printUsage();
			}
			k = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			numNodes = atoi(argv[i + 1]);
			
			if (numNodes == 0) {
				cerr << "Must have at least one node in graph.\n";
				printUsage();
			}
			i++;
		}
		else if (strcmp(argv[i], "-b") == 0) {
			if (inputType) {
				cerr << "Only one input is allowed.\n";
				printUsage();
			}
			inputType = true;
			
			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			inputBitstring = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			if (inputType) {
				cerr << "Only one input type is allowed.\n";
				printUsage();
			}
			inputType = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			inputDecimalNum = std::stoull(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-o") == 0) {
			if (o) {
				cerr << "Only one output file is allowed.\n";
				printUsage();
			}
			o = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			outputFile = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-n") == 0) {
			if (n) {
				cerr << "Only one names file is allowed.\n";
				printUsage();
			}
			n = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			namesFile = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printHelp();
		} else if (strcmp(argv[i], "-u") == 0) {
			if (matrixType) {
				cerr << "Only one matrix type allowed.\n";
				printUsage();
			}
			isUpper = true;
			matrixType = true;
		} else if (strcmp(argv[i], "-l") == 0) {
			if (matrixType) {
				cerr << "Only one matrix type allowed.\n";
				printUsage();
			}
			isUpper = false;
			matrixType = true;
		} else if (strcmp(argv[i], "-t") == 0) {
			if (title) {
				cerr << "Only one title allowed.\n";
				printUsage();
			}

			title = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			graphTitle = argv[i + 1];
			i++;
		}
		else {
			cerr << "Unrecognized argument: " << argv[i] << "\n";
			printUsage();
		}
	}

	//Check if k, inputType, and output file were selected
	if (!(k && inputType && o))
		printUsage();

	//Convert decimal input to bitstring.
	if (inputBitstring == "") {
		if (numNodes > 11)
			cerr << DECIMAL_INPUT_WARNING;
		
		inputBitstring = toBitString(inputDecimalNum, numNodes);
	}
	inputBitstring = appendLeadingZeros(inputBitstring, numNodes);
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

void printUsage() {
	cerr << USAGE;
	exit(EXIT_SUCCESS);
}

//Contains useful information about the assumptions made in the program.
void printHelp() {
	std::cout << USAGE 
			  << "You must specify the number of nodes with -k\n"
			  << "Currently number of nodes is limited to 11 if decimal input is used\n"
			  << "This is because k=12 requires 66 bits to store the decimal input\n"
			  << "You must specify either a bitstring or a decimal input with -b or -d\n"
			  << "-u and -l specify if input is upper or lower triangular row major\n"
			  << "Lower triangular row major is assumed\n"
			  << "You must specify an output file name with -o\n"
			  << "Please do not include file extension for output. The program with generate a .pdf\n"
			  << "If no names file is selected, nodes will be named n0..n(k-1)\n"
			  << "Names file parsing assumes one name per line\n"
			  << "You may specify a title with -t\n"
			  << "If less names than nodes, additional nodes will be named ni with i being node #\n"
			  << "If more names than nodes, additional names will become orphan nodes\n";
	exit(EXIT_SUCCESS);
}

/**Creates .dot file from given input.
 * First, node names are listed.
 * If a names file was specified, the nodes are labeled with their name.
 * If the names file has a different number of names than k,
 * extra names become isolated nodes and additional nodes aren't labeled.
 * Then the edges are written to the file.
 * */
void createDotfileFromBit(int numNodes, const string& inputBitstring, const string& outputFile, const string& namesFile, bool isUpper, const string& graphTitle) {
	unsigned int size = inputBitstring.size();
	int finalBitstringSize = (numNodes * (numNodes - 1)) / 2;
	if (finalBitstringSize != size) {
		cerr << "Input size does not match number of nodes.\n"
			 << "Expected Bitstring Size given k = " << numNodes << " is:  "
			  << finalBitstringSize << "\nInput Bitstring Size: " << inputBitstring.size() << "\n";
		exit(EXIT_FAILURE);
	}

	ofstream outfile;
	stringstream ss;
	ss << outputFile << ".dot";
	outfile.open(ss.str());
	if (!outfile) {
		cerr << "Unable to create Dot File. Temporary .dot file could not be created.\n";
		exit(EXIT_FAILURE);
	}
	
	outfile << "graph {\n";

	if (namesFile != "") {
		std::ifstream infile;
		infile.open(namesFile);
		if (infile) {
			string nodeName;
			int i = 0;
			while (std::getline(infile, nodeName)) {
				outfile << 'n' << i << " [label=\"" << nodeName << "\", pos=\"" << getPos(i, numNodes) << "!\"]" << NODE_ARGS<< ";\n";
				i++;
			}
			if (i < numNodes) {
				cerr << "Warning: Less nodes in names file than -k.\n"
					 << "Number of nodes: " << numNodes << " Names file number of nodes: " << i << "\n";
			}
			else if (i > numNodes) {
				cerr << "Warning: More nodes in names file than -k.\n"
			         << "Number of nodes: " << numNodes << " Names file number of nodes: " << i << "\n";								
			}
			infile.close();
		} else {
			cerr << "Could not open name file\n";
		}
	} else {
		for (int i = 0; i < numNodes; i++)
			outfile << 'n' << i << "[pos=\"" << getPos(i, numNodes) <<  "!\"]\n";
	}

	writeEdges(outfile, inputBitstring, numNodes, isUpper);
	if (graphTitle != "") {
		outfile << "labelloc=\"b\";\n"
				<< "label=" << graphTitle << '\n';
	}
	outfile << "}";
	outfile.close();
}

//Wrapper function to choose edge writing function based on matrix representation.
void writeEdges(ofstream& outfile, const string& inputBitstring, int numNodes, bool isUpper) {
	if (isUpper)
		writeEdgesUpper(outfile, inputBitstring, numNodes);
	else
		writeEdgesLower(outfile, inputBitstring, numNodes);
}

string getPos(int i, int numNodes) {
	stringstream ss;
	ss << RADIUS_SCALING * numNodes * cos(2 * PI / numNodes * i) << ", " << RADIUS_SCALING * numNodes * sin(2 * PI / numNodes * i);
	return ss.str();
}

//Assuming row major
void writeEdgesUpper(ofstream& outfile, const string& inputBitstring, int numNodes) {
	size_t size = inputBitstring.size();
	unsigned int i = 0, j = i + 1;	
	for (size_t k = 0; k < size; k++) {
		if (inputBitstring[k] == '1')
			outfile << "n" << i << " -- " << "n" << j << EDGE_ARGS << "\n";
		else if (inputBitstring[k] != '0')
			cerr << "Unknown input: " << inputBitstring[k] << " in input bitstring.\n";

		j++;
		if (j == numNodes) {
			i++;
			j = i + 1;
		}
	}
}
//Assuming row major
void writeEdgesLower(ofstream& outfile, const string& inputBitstring, int numNodes) {
	size_t size = inputBitstring.size();
	unsigned int i = 1, j = 0;
	for (size_t k = 0; k < size; k++) {
		if (inputBitstring[k] == '1')
			outfile << "n" << i << " -- " << "n" << j << EDGE_ARGS << "\n";
		else if (inputBitstring[k] != '0')
			cerr << "Unknown input: " << inputBitstring[k] << " in input bitstring.\n";
		j++;
		if (j == i) {
			i++;
			j = 0;
		}
	}
}

void runGraphviz(const string& filename) {
	stringstream ss;
	ss << "neato -n -Tpdf " << filename << ".dot -o" << filename << ".pdf";
	system(ss.str().c_str());
	//std::remove((filename + ".dot").c_str());
}