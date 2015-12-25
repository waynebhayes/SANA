#include "report.hpp"

#include "measures/EdgeCorrectness.hpp"
#include "measures/InducedConservedStructure.hpp"
#include "measures/SymmetricSubstructureScore.hpp"
#include "utils/utils.hpp"

void makeReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, ofstream& stream) {
  int numCCsToPrint = 3;

  stream << endl << currentDateTime() << endl << endl;

  stream << "G1: " << G1.getName() << endl;
  G1.printStats(numCCsToPrint, stream);
  stream << endl;

  stream << "G2: " << G2.getName() << endl;
  G2.printStats(numCCsToPrint, stream);
  stream << endl;

  stream << "Method: " << method->getName() << endl;
  method->describeParameters(stream);

  stream << endl << "execution time = " << method->getExecTime() << endl;

  stream << endl << "Scores:" << endl;
  M.printMeasures(A, stream);
  stream << endl;

  Graph CS = A.commonSubgraph(G1, G2);
  stream << "Common subgraph:" << endl;
  CS.printStats(numCCsToPrint, stream);
  stream << endl;

  int tableRows = min(5, CS.getConnectedComponents().size())+2;
  vector<vector<string> > table(tableRows, vector<string> (8));

  table[0][0] = "Graph"; table[0][1] = "n"; table[0][2] = "m"; table[0][3] = "alig-edges";
  table[0][4] = "indu-edges"; table[0][5] = "EC";
  table[0][6] = "ICS"; table[0][7] = "S3";

  table[1][0] = "G1"; table[1][1] = to_string(G1.getNumNodes()); table[1][2] = to_string(G1.getNumEdges());
  table[1][3] = to_string(A.numAlignedEdges(G1, G2)); table[1][4] = to_string(G2.numNodeInducedSubgraphEdges(A.getMapping()));
  table[1][5] = to_string(M.eval("ec",A));
  table[1][6] = to_string(M.eval("ics",A)); table[1][7] = to_string(M.eval("s3",A));

  for (int i = 0; i < tableRows-2; i++) {
    const vector<ushort>& nodes = CS.getConnectedComponents()[i];
    Graph H = CS.nodeInducedSubgraph(nodes);
    Alignment newA(nodes);
    newA.compose(A);
    table[i+2][0] = "CCS_"+to_string(i); table[i+2][1] = to_string(H.getNumNodes());
    table[i+2][2] = to_string(H.getNumEdges());
    table[i+2][3] = to_string(newA.numAlignedEdges(H, G2));
    table[i+2][4] = to_string(G2.numNodeInducedSubgraphEdges(newA.getMapping()));
    EdgeCorrectness ec(&H, &G2);
    table[i+2][5] = to_string(ec.eval(newA));
    InducedConservedStructure ics(&H, &G2);
    table[i+2][6] = to_string(ics.eval(newA));
    SymmetricSubstructureScore s3(&H, &G2);
    table[i+2][7] = to_string(s3.eval(newA));
  }

  stream << "Common connected subgraphs:" << endl;
  printTable(table, 2, stream);
  stream << endl;
}

void saveReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, string reportFile) {
  string G1Name = G1.getName();
  string G2Name = G2.getName();
  if (reportFile == "") {
    reportFile = "alignments/" + G1Name + "_" + G2Name + "/"
    + G1Name + "_" + G2Name + "_" + method->getName() + method->fileNameSuffix(A);
    addUniquePostfixToFilename(reportFile, ".txt");
    reportFile += ".txt";
  }else{
    string location = reportFile.substr(0, reportFile.find_last_of("/"));
    if (location != reportFile) {
      uint lastPos = 0;
      while(not folderExists(location)){//Making each of the folders, one by one.
        createFolder(location.substr(0, location.find("/", lastPos)));
        lastPos = location.find("/", location.find("/", lastPos)+1);
      }
    }
  }

  ofstream outfile;
  outfile.open(reportFile.c_str());
  
  if(not outfile.is_open()){
    cerr << "Problem saving file to specified location. Saving to sana program file." << endl;
    reportFile = reportFile.substr(reportFile.find_last_of("/")+1);
    outfile.open(reportFile.c_str());
  }

  cerr << "Saving report as \"" << reportFile << "\"" << endl;
  A.write(outfile);
  makeReport(G1, G2, A, M, method, outfile);
  outfile.close();
}
