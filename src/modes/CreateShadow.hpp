#ifndef CREATESHADOWMODE_HPP_
#define CREATESHADOWMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"

using namespace std;

class CreateShadow : public Mode {
public:

/* Format: 
The shadow file is passed with the -fshadow argument
It should contain the following, in order
(can be separated by any form of white space, including line breaks)

- file where the shadow graph should be stored, in .gw format with weights
- number of dummy nodes (per color, if more than one color)
- k, the number of graphs
- k unweighted graph files, in any supported format
- 0 or 1, depending on whether alignments are given
- if alignments are given, k alignment files, in named-pairs format
- 0 or 1, depending on whether colors are given
- if colors are given, k color files, in named-pairs format (not all nodes need to be listed)
- if colors are given, the file where the shadow graph colors should be stored

the graphs have to be passed in the same order between iterations
*/
    void run(ArgumentParser& args);
    string getName();
private:
    void createShadow(const string& outFile, uint numDummies,
                      const vector<string>& graphFiles, 
                      bool hasAligs, const vector<string>& aligFiles,
                      bool hasColors, const vector<string>& colorFiles, const string& outColorFile);

};

#endif /* CREATESHADOWMODE_HPP_ */
