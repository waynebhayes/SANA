/* shrinknetwork.h
Author: Jialu Hu
Date: 18.06.2012*/

#ifndef SHRINKNETWORK_H_
#define SHRINKNETWORK_H_

#include <unordered_map>
#include <iostream>
#include <fstream>

class ShrinkNetwork
{
private:
  std::string listfilename;
  std::string networkfilename;
  std::string outfilename;
  std::string blastmfilename;
  std::unordered_map<std::string,bool> nodemap;
public:
  ShrinkNetwork(std::string&);
  bool readNodeList();
  bool readNetwork();
  bool run();
};

ShrinkNetwork::ShrinkNetwork(std::string& s)
{
	listfilename.append(s);
	listfilename.append(".list");
	networkfilename.append(s);
	networkfilename.append(".txt");
	outfilename.append(s);
	outfilename.append(".tab");
	blastmfilename.append(s);
	blastmfilename.append(".nbm");
	
}

bool ShrinkNetwork::readNodeList()
{
  std::ifstream input(listfilename.c_str());
  std::string line;
  std::string node;
  if(!input.is_open())
  {
    std::cerr <<"Input file cannot be opened!"<<std::endl;
    return false;
  }
  while(std::getline(input,line))
  {
    std::stringstream streamline(line);
    streamline >> node;
    nodemap[node]=false;
  }
  input.close();
  return true;
}

bool ShrinkNetwork::readNetwork()
{
  std::ifstream input(networkfilename.c_str());
  std::ofstream output(outfilename.c_str());
  std::ofstream outputbnm(blastmfilename.c_str());
  std::string line;
  std::string protein1,protein2;
  std::unordered_map<std::string,int> interactionmap;
  float value;
  if(!input.is_open())
  {
    std::cerr <<"Input file cannot be opened!"<<std::endl;
    return false;
  }
  output << "INTERACTOR_A\tINTERACTOR_B\n"; 
  while(std::getline(input,line))
  {
    std::stringstream streamline(line);
    streamline >> protein1 >> protein2 >> value;
    if(nodemap.find(protein1)!=nodemap.end() &&
       nodemap.find(protein2)!=nodemap.end())
    {
	  std::string keystr;
	  if(protein1.compare(protein2)>0)
	  {
		  keystr.append(protein2);keystr.append(protein1);
	  }
	  else if(protein1.compare(protein2)<0)
	  {
		  keystr.append(protein1);keystr.append(protein2);
	  }
	  else
	  {
		  continue;
	  }
	  if(interactionmap.find(keystr)!=interactionmap.end()) continue;
	  interactionmap[keystr] = 1;
      output << protein1 <<"\t"<<protein2 << std::endl;
      outputbnm << protein1 <<"\t"<<protein2 <<"\t" << value << std::endl;
      nodemap[protein1]=true;
      nodemap[protein2]=true;
    }
  }
  input.close();
  output.close();
  outputbnm.close();
  return true;
}

bool ShrinkNetwork::run()
{
  readNodeList();
  readNetwork();
  return true;
}
#endif //SHRINKNETWORK_H_
