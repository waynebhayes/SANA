/* score_model.h
Author: Jialu Hu
Data: 26.07.2012*/

#ifndef SCORE_MODEL_H_
#define SCORE_MODEL_H_
#include <array>
#include <iostream>
#include <fstream>
#include "verbose.h"

class Score_Model
{
private:
  std::array<unsigned long,NUM_PRO_BARS> nullmodel;
  std::array<unsigned long,NUM_PRO_BARS> homomodel;
  std::array<double,  NUM_PRO_BARS> nullmodel_pro;
  std::array<float,   NUM_PRO_BARS> homomodel_pro;
  std::string nullfile,homofile, outputfile;
  unsigned long sum_null,sum_homo;
public:
  Score_Model(std::string&,std::string&,std::string&);
  ~Score_Model(){};
  void readDistribution(bool blastp);
  int getIndex(double);
  int getIndex_bit(float);
  void calculatePro();
  void run(bool blastp);
};
Score_Model::Score_Model(std::string& s1,std::string& s2,std::string& s3)
:sum_null(0)
,sum_homo(0)
{
  homofile= s1;
  nullfile= s2;
  outputfile= s3;
  nullmodel.fill(0);
  homomodel.fill(0);
  nullmodel_pro.fill(0);
  homomodel_pro.fill(0);
}
int Score_Model::getIndex(double evalue)
{
  int ind=0;
  if(evalue>1.0e-180)
  {
    ind=ceil(log10(evalue)/NUM_PRO_INTERVAL)+NUM_OFFSET_BARS;
  }
  return ind;
}
int Score_Model::getIndex_bit(float bitscore)
{
	int ind=0;
	if(bitscore < 940.0)
	{
		ind=bitscore/10;
	}else
	{
		ind=NUM_PRO_BARS-1;
	}
	return ind;
}
void Score_Model::readDistribution(bool bscore)
{
  std::ifstream input1(homofile.c_str());
  std::ifstream input2(nullfile.c_str());
  std::string line;
  std::string protein1,protein2;
  float bitscore;
  double evalue;
  if(!input1.is_open() || !input2.is_open())
  {
    std::cerr <<"One of input files cannot be opened!"<<std::endl;
    return;
  }
  while(std::getline(input1,line))
  {
    std::stringstream streamline(line);
    streamline >> protein1 >> protein2 >>  bitscore >> evalue;

    int num;
	if(!bscore)
	  num=getIndex(evalue);
	else
	  num=getIndex_bit(bitscore);
    homomodel[num]++;
    sum_homo++;
  }
  while(std::getline(input2,line))
  {
    std::stringstream streamline(line);
    int num;
    if(!bscore)
    {
		streamline >> evalue;
		num=getIndex(evalue);
	}else
	{   streamline >> bitscore;
		num=getIndex_bit(bitscore);
	}    
    nullmodel[num]++;
    sum_null++;
  }
}
void Score_Model::calculatePro()
{
  std::ofstream output(outputfile.c_str(),std::ios_base::out|std::ios_base::app);
  output <<"#Distribution of "<<nullfile <<" and "<< homofile<<":"<<std::endl;
  for(int i=0;i<NUM_PRO_BARS;++i)
  {
    nullmodel_pro[i]=static_cast<float>(nullmodel[i])/sum_null;
    homomodel_pro[i]=static_cast<float>(homomodel[i])/sum_homo;
    output << nullmodel_pro[i] << " " << homomodel_pro[i] << std::endl;
  }
  output.close();
}
void Score_Model::run(bool bscore)
{
  readDistribution(bscore);
  calculatePro();
}
#endif //SCORE_MODEL_H_
