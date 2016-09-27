#ifndef SEED_MATRIX_HPP
#define SEED_MATRIX_HPP

#include <limits.h>
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <exception>
#include <unordered_set>

#include <chrono>

#include "../measures/MeasureCombination.hpp"

using namespace std;

//typedef struct {
class node_t {
public:
  float sim;
  ushort node;
  //bool operator < (const node_t & left, const node_t & right){
  bool operator < (const node_t & right) const{
    return (this->sim < right.sim);
  }
};
/*
bool operator < (const node_t & left, const node_t & right){
  return (left.sim < right.sim);
}
*/
class seed_t {
public:
  ushort left_node;
  vector<node_t> * pairs;
  float top;
  uint start;
  uint end;
  bool operator < (const seed_t & right) const{
    return (this->top < right.top);
  }
  float getTop(){
    return top;
  }
  uint getStart(){
    int s = start;
    return s;
  }
  uint getEnd(){
    int e = end;
    return e;
  }
};



class SeedMatrix{
public:
  SeedMatrix(MeasureCombination * MC, double delta, bool isMaxHeap, std::unordered_set<ushort> & lex, std::unordered_set<ushort> & rex);
  ~SeedMatrix();
  std::pair<ushort,ushort> pop_uniform();
  void init_column_vector(vector<vector<float> > & sims);
  //void save();
  //void load();
  /*
    bool empty();
  */
protected:
  
private:
  vector<vector<float> > sims;
  uint init_ptr;
  float delta;
  bool isMaxHeap;

  void init_column_vector();
  vector<node_t> * create_node(ushort node_num);
  void delete_node(seed_t & selected);
  static uint bin_search(vector<node_t> & vec, float val, uint i, uint j);
  //void delete_row(int i);

  void debug();

  vector<seed_t> column_vector;

  std::mt19937 rng;
  std::unordered_set<ushort> & left_exclude;
  std::unordered_set<ushort> & right_exclude;

};






#endif
