#include <iostream> //Required for console output
#include <string> //Used to store character string data that represents the information of the options and arguments of SANA.
#include <vector> //Used as an array structure holding all options/arguments in grouped collections.
#include <array> //Used to store 6 fields of information in supportedArguments datatype defined below.
#include <iomanip> //Used to format the text console output when ./sana -h or ./sana --help is executed.
#include <sstream> //Used to format the text console output when ./sana -h or ./sana --help is executed.
#include <fstream> //Used to access a text file called helpOutput which contains information useful to the user to understand how to run SANA on the command line.

using namespace std;

extern vector<string> stringArgs; //We load the argument names of supportedArguments (Declared below) into stringArgs, doubleArgs, boolArgs, and vectorArgs.
extern vector<string> doubleArgs; //Later, the information stored into these four data types get loaded into a map declared and defined in ArgumentParser.hpp/.cpp
extern vector<string> boolArgs;
extern vector<string> vectorArgs;
extern vector<array<string, 6>> supportedArguments; //Initially stores information on options/arguments in SANA.

void validateAndAddArguments(); //Checks to make sure fields inside supportedArguments are properly filled out and adds them to the four Args datatypes.
void printAllArgumentDescriptions(); //Used to generate the help text when ./sana -h or ./sana --help is invoked from command line.
string printItem(const array<string, 6> &item);	//Helper function of printAllArgumentDescriptions() that prints a single argument to the console.
string formatDescription(string description); //helper function of printItem() that helps with the formatting of console output.
bool hasNewLines(const string &item4); //helper function of formatDescription() that evaluates whether a description of an argument should be formatted on many lines.
string formatWithNewLines(const string &item4); //helper function of formatDescription() that outputs a description of an argument on multiple lines when hasNewLines() evaluates to true.
