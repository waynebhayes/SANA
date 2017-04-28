/* mask_whitespace.h
Author: Jialu Hu
Data: 23.07.2012*/

#ifndef MASK_WHITESPACE_H_
#define MASK_WHITESPACE_H_

#include <iostream>
#include <fstream>
#include <string>
class MaskWhiteSpace
{
public:
  std::string inputfilename;
  std::string outputfilename;
  MaskWhiteSpace(std::string resultfolder)
  {
	inputfilename.append(resultfolder);
	outputfilename.append(resultfolder);
	inputfilename.append("clusters.txt");
	outputfilename.append("alignment_isorankn.data");
  }
  bool run();
};

bool
MaskWhiteSpace::run()
{
  std::ifstream input(inputfilename.c_str());
  std::ofstream output(outputfilename.c_str());
  std::string line;
  while(std::getline(input,line))
  {
    std::stringstream streamline(line);
    std::string str;
    std::vector<std::string> record;
    while(streamline.good())
    {
      streamline >> str;
      record.push_back(str);
    }
    record.pop_back();
    std::vector<std::string>::iterator it=record.begin();
    unsigned i,len=record.size();
    for(i=0;i<len-1;i++)
    {
      output << record[i]<<"\t";
      ++it;
    }
    output << record[i]<<std::endl;
  }
  return true;
}
#endif //MASK_WHITESPACE_H_
