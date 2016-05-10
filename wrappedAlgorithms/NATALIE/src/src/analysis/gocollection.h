/* 
 * gocollection.h
 *
 *  Created on: 15-jun-2011
 *      Author: M. El-Kebir
 */

#ifndef GOCOLLECTION_H_
#define GOCOLLECTION_H_

#include <string>
#include <set>
#include <map>
#include <fstream>
#include <sstream>

namespace nina {
namespace gna {

class GoCollection
{
public:
  typedef std::set<std::string> GoTermSet;
  typedef std::map<std::string, GoTermSet> GoTermsMap;

public:
  GoCollection() {}
  ~GoCollection() {}
  bool init(const std::string& goFileName);
  GoTermSet getGoTerms(const std::string& label) const;
  int getGoTermsCount(const std::string& label) const;

private:
  GoTermsMap _goTermsMap;
};

inline bool GoCollection::init(const std::string& goFileName)
{
  std::ifstream inFile(goFileName.c_str());
  if (!inFile.good())
  {
    std::cerr << "Could not open '" 
      << goFileName << "' for reading" << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(inFile, line))
  {
    std::stringstream ss(line);

    std::string label;
    ss >> label;

    GoTermSet& termSet = _goTermsMap[label];

    std::string term;
    while (ss >> term)
      termSet.insert(term);
  }

  return true;
}

inline GoCollection::GoTermSet GoCollection::getGoTerms(const std::string& label) const
{
  GoTermsMap::const_iterator it = _goTermsMap.find(label);
  if (it == _goTermsMap.end())
  {
    // return the empty set
    return GoTermSet();
  }
  else
  {
    return it->second;
  }
}

inline int GoCollection::getGoTermsCount(const std::string& label) const
{
  GoTermsMap::const_iterator it = _goTermsMap.find(label);
  if (it == _goTermsMap.end())
  {
    return 0;
  }
  else
  {
    return static_cast<int>(it->second.size());
  }
}

} // namespace gna
} // namespace nina

#endif /* GOCOLLECTION_H_ */
