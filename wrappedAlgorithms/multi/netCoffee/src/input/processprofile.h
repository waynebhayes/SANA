/* processprofile.h
Author: Jialu Hu
Date: 18.09.2012*/
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

template<typename Option>
class ProcessProfile
{
private:
  typedef std::multimap<std::string,std::string> ProfileMap;
  typedef std::vector<std::string> FileNames;
  std::string profile;
  ProfileMap profileMap;
public:
  ProcessProfile(std::string);
  bool readProfile();
  bool getArray(std::string&,FileNames&);
  bool getOption(Option&);
  bool getKey(std::string&,std::string&);
};

template<typename Option>
ProcessProfile<Option>::ProcessProfile(std::string filename)
{
  profile=filename;
  readProfile();
}

template<typename Option>
bool
ProcessProfile<Option>::readProfile()
{
  std::ifstream input(profile.c_str());
  std::string line;
  std::string mkey,mvalue;
  if(!input.is_open())
  {
    std::cerr << profile<<" can't be opened!"<<std::endl;
  }
  while(std::getline(input,line))
  {
    if(line[0]=='#')continue;
    std::stringstream lineStream(line);
    lineStream >> mkey >> mvalue;
    profileMap.insert(std::pair<std::string,std::string>(mkey,mvalue));
  }
  input.close();
  return true;
}

template<typename Option>
bool
ProcessProfile<Option>::getArray(std::string& mykey,FileNames& files)
{
  auto range=profileMap.equal_range(mykey);
  files.clear();
  for(auto it=range.first;it!=range.second;++it)
  {
    files.push_back(it->second);
  }
  return true;
}

template<typename Option>
bool
ProcessProfile<Option>::getOption(Option& myoption)
{
  std::string mykey="network";
  getArray(mykey,myoption.networkfiles);
  mykey="efile";
  getArray(mykey,myoption.blastfiles);
  mykey="nfile";
  getArray(mykey,myoption.nullfiles);
  mykey="gofile";
  getArray(mykey,myoption.associationfiles);
  mykey="alignmentfile";
  getKey(mykey,myoption.alignmentfile);
  mykey="avefunsimfile";
  getKey(mykey,myoption.avefunsimfile);
  mykey="scorefile";
  getKey(mykey,myoption.scorefile);
  mykey="logfile";
  getKey(mykey,myoption.logfile);
  return true;
}

template<typename Option>
bool
ProcessProfile<Option>::getKey(std::string& mykey,std::string& value)
{
  auto it=profileMap.find(mykey);
  if(it!=profileMap.end())
  {
    value = it->second;
  }
  else
  {
    std::cerr <<"Option "<<mykey<<" is not correctly specified!"<<std::endl;
  }
  return true;
}
