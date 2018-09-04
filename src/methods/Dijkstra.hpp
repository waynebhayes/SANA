#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <random>


#include <chrono>

#include "Method.hpp"
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/SkipList.hpp"
//#include "../utils/SeedMatrix.hpp"

using namespace std;

class Dijkstra: public Method {

public:
  Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC, double delta);
    
  ~Dijkstra();

  Alignment run();
  void describeParameters(ostream& stream);
  string fileNameSuffix(const Alignment& A);

private:

  Graph* G1;
  Graph* G2;
  MeasureCombination* MC;

  uint n1;
  uint n2;

  //vector<vector<bool> > G1AdjMatrix;
  //vector<vector<bool> > G2AdjMatrix;
  vector<vector<uint> > G1AdjLists;
  vector<vector<uint> > G2AdjLists;

  vector<uint> A;

  vector<vector<float> > sims;

  bool implementsLocking(){ return false; }
    
  //beginning of Grady's private variables
  string outputAName;
  unsigned int max_nodes;
    
  std::unordered_set<uint> G1_exclude;
  std::unordered_set<uint> G2_exclude;    
  //std::unordered_set<uint> current_neighbors;

  double delta;
  static constexpr double EPSILON = 0.0000001;
  //SeedMatrix seed_mat;
  SkipList seed_queue;
  SkipList neighbor_queue;
  unsigned int nodes_aligned;

  /* Member functions */
    
  void make_seed_queue();
  std::pair <uint, uint> get_seed(Graph* G1, Graph* G2);
  std::pair <uint, uint> best_pair(SkipList & pq) throw(QueueEmptyException);
  void update_neighbors(std::pair <uint, uint> & seed_pair);
  void best_neighbors(std::pair <uint, uint> & seed_pair, vector<uint> & G1_neighbors, vector<uint> & G2_neighbors);
  vector<uint> exclude_nodes(vector<uint> & v_in, std::unordered_set<uint> & exclusion_set);

};

#endif
