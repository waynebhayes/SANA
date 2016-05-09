/* 
 * analyseGO.cpp
 *
 *  Created on: 9-jun-2011
 *      Author: M. El-Kebir
 */

#include <lemon/arg_parser.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "analysis/gocollection.h"
#include "analyzego.h"
#include "verbose.h"

using namespace nina;
using namespace nina::gna;

bool AnalyseGO::init(const std::string& go1,
                     const std::string& go2,
                     const std::string& alignment)
{
  if (!_goG1.init(go1) || !_goG2.init(go2))
    return false;

  std::ifstream inFile(alignment.c_str());
  if (!inFile.good())
  {
    std::cerr << "Could not open '" 
      << alignment << "' for reading" << std::endl;
    return false;
  }

  std::string label1, label2;
  while (inFile >> label1 >> label2)
  {
    _labelG1[label1] = label2;
    _labelG2[label2] = label1;
  }

  return true;
}

void AnalyseGO::compute(int k, int& correct, double& pValue) const
{
  correct = 0;
  for (LabelMapIt it = _labelG1.begin(); it != _labelG1.end(); it++)
  {
    std::string label1 = it->first, label2 = it->second;

    GoCollection::GoTermSet commonGoTerms, goTerms1, goTerms2;
    goTerms1 = _goG1.getGoTerms(label1);
    goTerms2 = _goG2.getGoTerms(label2);

    int count = 0;
    GoCollection::GoTermSet::const_iterator termIt1 = goTerms1.begin();
    GoCollection::GoTermSet::const_iterator termIt2 = goTerms2.begin();
    while (termIt1 != goTerms1.end() && termIt2 != goTerms2.end())
    {
      if (*termIt1 < *termIt2) termIt1++;
      else if (*termIt1 > *termIt2) termIt2++;
      else
      {
        count++; termIt1++; termIt2++;
      }
    }

    if (count >= k)
    {
      correct++;
      if (g_verbosity > VERBOSE_NONE)
      {
        std::cout << label1 << '\t' << label2 << '\t' << count << std::endl;
      }
    }
  }
}

int main(int argc, char** argv)
{
  lemon::ArgParser ap(argc, argv);

  int k;
  std::string alignmentFile, goFile1, goFile2;
  int verbosityLevel = static_cast<int>(VERBOSE_NONE);

  ap.refOption("g1", "GO file #1", goFile1, true)
    .refOption("g2", "GO file #2", goFile2, true)
    .refOption("a", "Alignment", alignmentFile, true)
    .refOption("k", "Number of shared GO terms", k, true)
    .refOption("v", "Specifies the verbosity level:\n"
                    "     0 - No output (default)\n"
                    "     1 - Only necessary output\n"
                    "     2 - More verbose output\n"
                    "     3 - Debug output", verbosityLevel, false)
    .synonym("-verbosity", "v");
  ap.parse();

  g_verbosity = static_cast<VerbosityLevel>(verbosityLevel);

  AnalyseGO analyze(k);
  if (!analyze.init(goFile1, goFile2, alignmentFile))
    return 1;

  std::cout << "Computing GO-based score... " << std::flush;
  double pVal = 1;
  int score = 0;
  analyze.compute(k, score, pVal);
  std::cout << "Score = " << score << ", p-val = " << pVal << std::endl;

  return 0;
}
