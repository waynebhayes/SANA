#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <stdlib.h>
#include <tgmath.h>
#include "swapEndian.hpp"
#include <boost/unordered_map.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

# define PI 3.14159265358979323846

namespace io = boost::iostreams;
using std::vector;
using std::string;
using std::istream;
using std::ofstream;
using std::ostringstream;
using std::cout;

//Fancy fast pow2 function from fastonebigheader.h
static inline float fastpow2 (float p)
{
  float offset = (p < 0) ? 1.0f : 0.0f;
  float clipp = (p < -126) ? -126.0f : p;
  int w = clipp;
  float z = clipp - w + offset;
  union { uint32_t i; float f; } v = { static_cast<uint32_t> ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };
  return v.f;
}

struct LevelInfo{
  vector<string> vnames;
  vector<double> spectrum;
  vector<double> signature;
  double density;
  LevelInfo(){};
  LevelInfo(vector<string> v, vector<double> s, double d, vector<double> sig) :
    vnames(v), spectrum(s), density(d), signature(sig){};
};

typedef boost::unordered_map<string, vector<LevelInfo> > spectramap;

//Helpers for Reading the GZip
int readInt(istream& in)
{
  int num = sizeof(int32_t)/sizeof(char);
  char* s = new char [num];
  in.read(s, num);
  int32_t d = *((int32_t*)s);
  swap_endian(d);
  delete[] s;
  return d;
}

double readDouble(istream& in)
{
  int num = sizeof(int64_t)/sizeof(char);
  char* s = new char [num];
  in.read(s, num);
  int64_t d = *((int64_t*)s);
  swap_endian(d);
  double ans = *((double*)&d);
  delete[] s;
  return ans;
}

string readString(istream& in)
{
  int num = sizeof(int16_t)/sizeof(char);
  char* s = new char [num];
  in.read(s, num);
  int16_t d = *((int16_t*)s);
  swap_endian(d);
  delete[] s;
  s = new char [d+1];
  in.read(s, d);
  s[d]='\0';
  string ss = string(s);
  delete[] s;
  return ss;
}

double gaussianDensity(double* spec, int s, double omega, double sigsq, double norm)
{
  double d = 1e-30;
  for(int i=0; i<s; i++) 
    d += norm * fastpow2(1.442695040f * -((omega - spec[i]) * (omega - spec[i])) / sigsq);
  return d;
}

vector<double> ipsenMikhailovVector(double* spectrum, int s, double gamma)
{
  double c = 0;
  vector<double> dens;
  double ub = gamma>.1?2.01:(int)((2 + .1*gamma)*100)/100.0;
  double step = gamma<.1?.01:(int)(10*gamma)/100.0;
  double sigsq = 2.0*gamma*gamma;
  double norm = 1.0/sqrt(PI * sigsq);
  for(double i = 0; i < ub; i += step){
    double d = gaussianDensity(spectrum, s, i, sigsq, norm);
    c += d;
    dens.push_back(d);
  }
  for(int i=0; i < dens.size(); i++) dens[i] /= c;
  return dens;
}

//Read the signatures from the GZip file, and calculate the spectral signatures
spectramap loadSigs(string file)
{

  if(FILE *check = fopen(file.c_str(),"r")) {fclose(check);}
  else {std::cout << "file: " << file << " not found\n"; exit(0);}

  io::filtering_istream in;
  in.push(io::gzip_decompressor());
  in.push(io::file_descriptor_source(file));

  int numVerts = readInt(in);
  int numHops = readInt(in);

  spectramap map;

  for(int i=0; i < numVerts; i++)
  {
    vector<LevelInfo> levels;
    string vname = readString(in);
    for(int j=0; j < numHops; j++)
    {
      int l = readInt(in);
      int numnames = readInt(in);
      vector<string> vnames;
      vector<double> spectrum;
     
      for(int k=0; k < numnames; k++)
        vnames.push_back(readString(in));

      int spectrumSize = readInt(in);

      for(int k=0; k < spectrumSize; k++)
        spectrum.push_back(readDouble(in));

      double density = readDouble(in);
      LevelInfo m(vnames,spectrum,density,
                  ipsenMikhailovVector(&spectrum[0], spectrum.size(), .005));
      levels.push_back(m);
    }
    map[vname] = levels;
  }
  return map;
}

