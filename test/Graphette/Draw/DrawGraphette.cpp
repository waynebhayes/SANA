#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
//Requires c++11 for stoull
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <math.h>
#include "graphette2dotutils.h"

using std::cout;
using std::cerr;
using std::string;
using std::stringstream;
using std::ofstream;
using std::ifstream;
using std::vector;

void parseInput(int argc, char* argv[], int& numNodes, vector<string>& inputBitstrings, string& outputFile, string& namesFile, bool& isUpper, string& graphTitle, int& edgewidth);	
void printUsage();
void printHelp();
string toBitString(unsigned long long inputDecimalNum, int numNodes);
string appendLeadingZeros(const string& inputBitstring, int numNodes);

void createDotfileFromBit(int numNodes, const vector<string>& inputBitstring, const string& outputFile, const string& namesFile, bool isUpper, const string& graphTitle, int edgewidth);
string getPos(int i, int numNodes);
void writeEdges(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, bool isUpper, int edgewidth);
void writeEdgesUpper(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, int edgewidth);
void writeEdgesLower(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, int edgewidth);
void printGraphConversionInstruction(const string& fileName);

const int RADIUS_SCALING = 25;
const string USAGE = "USAGE: ./graphette2dot <-k number_of_nodes> <-b bitstring | -d decimal_representation> <-o output_filename> [-n input_filename] [-u | -l] [-t title] [-ew edge width] -h for verbose help\n";
const string NODE_ARGS = "shape = \"none\", fontsize = 24.0";
const string TITLE_ARGS = "fontsize = 24.0";
const string DECIMAL_INPUT_WARNING = "Warning. Decimal input was used with k > 11. Edge information may have been lost.\n";
const double PI  =3.141592653589793238463;
const vector<string> COLORS = {"black", "red", "lawngreen", "orange", "blue", "yellow", "indigo"};

int main(int argc, char* argv[]) {
	int numNodes = 0;
	vector<string> inputBitstrings;
	string outputFile = "", namesFile = "", graphTitle = "";
	int edgewidth = 1;
	//Defaults to lower row major bitstring/decimal interpretation
	bool isUpper = false;

	//Parses input passing variables by reference.
	parseInput(argc, argv, numNodes, inputBitstrings, outputFile, namesFile, isUpper, graphTitle, edgewidth);

	createDotfileFromBit(numNodes, inputBitstrings, outputFile, namesFile, isUpper, graphTitle, edgewidth);

	printGraphConversionInstruction(outputFile);

	return EXIT_SUCCESS;
}

/**
 * Parses command line input.
 * Doesn't allow for repeated inputs.
 * Prints usage and exits if invalid input is passed.
 * */
void parseInput(int argc, char* argv[], int& numNodes, vector<string>& inputBitstrings, string& outputFile, string& namesFile, bool& isUpper, string& graphTitle, int& edgewidth) {
	bool k = false, input = false, o = false, n = false, matrixType = false, title = false;
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

			input = true;
			
			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			inputBitstrings.push_back(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-d") == 0) {
			input = true;

			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}

			inputDecimalNum = std::stoull(argv[i + 1]);
			inputBitstrings.push_back(toBitString(inputDecimalNum, numNodes));

			if (numNodes > 11)
				cerr << DECIMAL_INPUT_WARNING;

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
		} else if (strcmp(argv[i], "-ew") == 0) {
			if (i + 1 >= argc) {
				cerr << "No following argument to " << argv[i] << '\n';
				printUsage();				
			}
			edgewidth = atoi(argv[i+1]);
			i++;
		}
		else {
			cerr << "Unrecognized argument: " << argv[i] << "\n";
			printUsage();
		}
	}

	//Check if k, input data, and output file were selected
	if (!(k && input && o))
		printUsage();

	for (string& bitstring : inputBitstrings) {
		bitstring = appendLeadingZeros(bitstring, numNodes);
	}
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
			  << "You must specify at least one bitstring or a decimal input with -b or -d\n"
			  << "-u and -l specify if all input is upper or lower triangular row major\n"
			  << "Lower triangular row major is assumed\n"
			  << "You must specify an output file name with -o\n"
			  << "Please do not include file extension for output. The program with generate a .dot\n"
			  << "If no names file is selected, nodes will be named 0, 1, ...., (k-1)\n"
			  << "Names file parsing assumes one name per line\n"
			  << "You may specify a title with -t\n"
			  << "You may specify an edge width with -ew. 1 is default\n"
			  << "If less names than nodes, additional nodes will be labeled by their index #\n"
			  << "If more names than nodes, additional names will be ignored\n";
	exit(EXIT_SUCCESS);
}

/**Creates .dot file from given input.
 * First, node names are listed.
 * If a names file was specified, the nodes are labeled with their name.
 * If the names file has a different number of names than k,
 * extra names become isolated nodes and additional nodes aren't labeled.
 * Then the edges are written to the file.
 * */
void createDotfileFromBit(int numNodes, const vector<string>& inputBitstrings, const string& outputFile, const string& namesFile, bool isUpper, const string& graphTitle, int edgewidth) {
	int finalBitstringSize = (numNodes * (numNodes - 1)) / 2;
	int size;
	for (string inputBitstring : inputBitstrings) {
		size = inputBitstring.size();
		if (finalBitstringSize != size) {
			cerr << "Input size does not match number of nodes.\n"
				<< "Expected Bitstring Size given k = " << numNodes << " is:  "
				<< finalBitstringSize << "\nInput Bitstring Size: " << size << "\n";
			exit(EXIT_FAILURE);
		}
	}

	ofstream outfile;
	stringstream ss;
	ss << outputFile << ".dot";
	outfile.open(ss.str());
	if (!outfile) {
		cerr << "Unable to create Dot File " << ss.str() << "\n";
		exit(EXIT_FAILURE);
	}
	
	outfile << "graph {\n";

	int i = 0;
	if (namesFile != "") {
		std::ifstream infile;
		infile.open(namesFile);
		if (infile) {
			string nodeName;
			while (std::getline(infile, nodeName) && i < numNodes) {
				outfile << 'n' << i << " [label=\"" << nodeName << "\", pos=\"" << getPos(i, numNodes) << "!\"" << NODE_ARGS << ";]\n";
				i++;
			}
			if (i < numNodes) {
				cerr << "Warning: Less nodes in names file than -k.\n"
					 << "Number of nodes: " << numNodes << " Names file number of nodes: " << i << "\n";
			}

			while (std::getline(infile, nodeName))
				i++;
			if (i > numNodes) {
				cerr << "Warning: More nodes in names file than -k.\n"
			         << "Number of nodes: " << numNodes << " Names file number of nodes: " << i << "\n";								
			}

			infile.close();
		} else {
			cerr << "Could not open name file\n";
		}
	}
	while (i < numNodes) {
		outfile << 'n' << i << "[label=\"" << i << "\", pos=\"" << getPos(i, numNodes) <<  "!\"" << NODE_ARGS << "]\n";
		i++;
	}

	writeEdges(outfile, inputBitstrings, numNodes, isUpper, edgewidth);
	if (graphTitle != "") {
		outfile << "labelloc=\"b\";\n"
				<< "label=\"" << graphTitle << "\"\n"
				<< TITLE_ARGS << '\n';
	}
	outfile << "}";
	outfile.close();
}

string getPos(int i, int numNodes) {
	stringstream ss;
	ss << RADIUS_SCALING * numNodes * cos(PI /2 - (2 * PI / numNodes * i)) << ", " << RADIUS_SCALING * numNodes * sin(PI / 2 - (2 * PI / numNodes * i));
	return ss.str();
}

//Wrapper function to choose edge writing function based on matrix representation.
void writeEdges(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, bool isUpper, int edgewidth) {
	if (isUpper)
		writeEdgesUpper(outfile, inputBitstrings, numNodes, edgewidth);
	else
		writeEdgesLower(outfile, inputBitstrings, numNodes, edgewidth);
}

//Assuming row major
void writeEdgesUpper(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, int edgewidth) {
	size_t size = inputBitstrings[0].size();
	int i = 0, j = 1, color = 0;
	string penwidth = "";
	if (edgewidth != 1) {
		penwidth = (", penwidth=" + std::to_string(edgewidth));
	}
	for (size_t k = 0; k < size; k++) {
		//Look at every string if they have an edge
		color = 0;
		for (string inputBitstring : inputBitstrings) {
			if (inputBitstring[k] == '1')
				outfile << "n" << i << " -- " << "n" << j << "[color=" << COLORS[color] << penwidth << "]" << "\n";
			else if (inputBitstring[k] != '0')
				cerr << "Unknown input: " << inputBitstring[k] << " in input bitstring.\n";

			color++;
			if (color > static_cast<int>(COLORS.size())) {
				cerr << "Too many graphs: Add colors to colors array" << std::endl;
				exit(EXIT_FAILURE);
			}
		}

		//iterate through pretend adjecency matrix
		j++;
		if (j == numNodes) {
			i++;
			j = i + 1;
		}
	}
}

//Assuming row major
void writeEdgesLower(ofstream& outfile, const vector<string>& inputBitstrings, int numNodes, int edgewidth) {
	size_t size = inputBitstrings[0].size();
	unsigned int i = 1, j = 0, color = 0;
	string penwidth = "";
	if (edgewidth != 1) {
		penwidth = (", penwidth=" + std::to_string(edgewidth));
	}
	for (size_t k = 0; k < size; k++) {
		//Look at every string if they have an edge
		color = 0;
		for (string inputBitstring : inputBitstrings) {
			if (inputBitstring[k] == '1')
				outfile << "n" << i << " -- " << "n" << j << "[color=" << COLORS[color] << penwidth << "]" << "\n";
			else if (inputBitstring[k] != '0')
				cerr << "Unknown input: " << inputBitstring[k] << " in input bitstring.\n";

			color++;
		}

		//iterate through pretend adjecency matrix
		j++;
		if (j == i) {
			i++;
			j = 0;
		}
	}
}

void printGraphConversionInstruction(const string& filename) {
	stringstream ss;
	ss << "neato -n -Tpdf " << filename << ".dot -o " << filename << ".pdf";
	std::cout << ss.str() << std::endl;
}
