/* 
 * analyseGO.h
 *
 *  Created on: 15-jun-2011
 *      Author: M. El-Kebir
 */

#ifndef ANALYSEGO_H_
#define ANALYSEGO_H_

#include <string>
#include <map>
#include <fstream>
#include "analysis/gocollection.h"

namespace nina {
namespace gna {

class AnalyseGO
{
private:
  /// Alignment map
  typedef std::map<std::string, std::string> LabelMap;
  typedef LabelMap::const_iterator LabelMapIt;

  LabelMap _labelG1;
  LabelMap _labelG2;
  GoCollection _goG1;
  GoCollection _goG2;
  /// The number of GO terms that should be shared
  int _k;
  /// The number of all possible node pairs in which both proteins are annotated with at least _k GO term
  int _p;
  /// The number of pairs in the alignment where both proteins are annotated with at least _k GO terms
  int _m1;
  /// The number of pairs (out of _p) where both proteins are annotated with at least _k common GO terms
  int _m2;

public:
  AnalyseGO(int k) {}
  ~AnalyseGO() {}
  bool init(const std::string& go1,
            const std::string& go2,
            const std::string& alignment);
  void compute(int k, int& correct, double& pValue) const;
};

} // namespace gna
} // namespace nina

#endif /* ANALYSEGO_H_ */
