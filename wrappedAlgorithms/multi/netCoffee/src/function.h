/* function.h
Author: Jialu Hu
Date: 28.07.2013*/

#ifndef FUNCTION_H_
#define FUNCTION_H_

int getIndex(double evalue)
{
  int ind=0;
  if(evalue>1.0e-180)
  {
    ind=ceil(log10(evalue)/NUM_PRO_INTERVAL)+NUM_OFFSET_BARS;
  }
  return ind;
}
int getIndex_bit(float bitscore)
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
/// Convert number types to string type.
template<typename TNum>
std::string
convert_num2str(TNum num)
{
	std::ostringstream buff;
	buff<<num;
	return buff.str();
}
#endif
