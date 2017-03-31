#include <string>
#include <algorithm>
#include <iostream>
#include "swapEndian.hpp"
#include "graph.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

using std::vector;
using std::string;
using std::cout;
namespace io = boost::iostreams;

struct LevelData
{
  vector<string> vnames;
  vector<double> spectrum;
  double density;
  LevelData(){};
  LevelData(vector<string> v, vector<double> s, double d):
    vnames(v), spectrum(s), density(d){};
};

typedef boost::unordered_map<string, vector<LevelData> > levelMap;

// writes the data so its backwards compatible
class GzWriter
{
  io::filtering_ostream out;
  public:
    GzWriter(string s)
    {out.push(io::gzip_compressor()); out.push(io::file_descriptor_sink(s));};
    void writeInt(int i) 
    {swap_endian(i); out.write(reinterpret_cast<const char*>(&i),sizeof(i));};
    void writeDouble(double d)
    {swap_endian(d); out.write(reinterpret_cast<const char*>(&d),sizeof(d));};
    void writeString(string s);
    void writeData(levelMap levelmap);
};

void GzWriter::writeString(string s)
{
  int16_t i = (int16_t)(s.size());
  swap_endian(i);
  out.write(reinterpret_cast<const char*>(&i),sizeof(i));
  out.write(reinterpret_cast<const char*>(&s[0]),s.size());
}

void GzWriter::writeData(levelMap levelmap)
{
  levelMap::iterator iter = levelmap.begin(),
  iend = levelmap.end();
  for(; iter != iend; iter++)
  {
    writeString(iter->first);
    for(int i=1;i<=(iter->second).size();i++)
    {
      writeInt(i);
      int sz = (iter->second)[i-1].vnames.size();
      writeInt(sz);
      for(int j=0;j<sz;j++)
        writeString((iter->second)[i-1].vnames[j]);
      sz = (iter->second)[i-1].spectrum.size();
      writeInt(sz);
      for(int j=0;j<sz;j++)
        writeDouble((iter->second)[i-1].spectrum[j]);
      writeDouble((iter->second)[i-1].density);
    }
  }
}

