
#include "Dijkstra.hpp"
#include "../utils/utils.hpp"

Dijkstra::Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC) :
  Method(G1, G2, "Dijkstra_"+MC->toString()),
  delta(0.05 + EPSILON), 
  seed_queue(delta, true), neighbor_queue(delta, true),
  nodes_aligned(0)
{

  this->G1 = G1;
  this->G2 = G2;
  this->MC = MC;

  n1 = G1->getNumNodes();
  n2 = G2->getNumNodes();

  //min of the two is the max nodes
  //that can be aligned
  max_nodes = n1 < n2 ? n1 : n2;
	
  G1->getAdjMatrix(G1AdjMatrix);
  G2->getAdjMatrix(G2AdjMatrix);
  G1->getAdjLists(G1AdjLists);
  G2->getAdjLists(G2AdjLists);

  A = vector<ushort> (n1);

  sims = MC->getAggregatedLocalSims();

}

/* Helper functions */

void Dijkstra::make_seed_queue(){
  
  std::cout << "seed queue begin" << std::endl;
  for(unsigned int i = 0; i < sims.size() && i < 5; ++i){
    for(unsigned int j = 0; j < sims[i].size() && j < 5; ++j){
      seed_queue.insert(sims[i][j], std::make_pair(i,j));
    }
  }/*
  for(unsigned int i = 0; i < sims.size(); ++i){
    for(unsigned int j = 0; j < sims[i].size(); ++j){
      seed_queue.insert(sims[i][j], std::make_pair(i,j));
    }
    }*/
  std::cout << "Seed Queue end " << std::endl;
  exit(13);
}

/*
 * this function should return an ORDERED pair of nodes having
 * the seed for G1 in position 0 and 
 * the seed for G2 in position 1
 * this function depends on a subroutine 
 * to determine the best seed pair,
 * which can be very slow O(|G1| * |G2|).
 * future: consider moving the reject loop to the subroutine
 * if there are no valid seeds, the subroutine should throw an error
 */
std::pair <ushort, ushort> Dijkstra::get_seed(Graph* G1, Graph* G2){
  //only one seed at a time
  return best_pair(seed_queue);
}

/*
 * This function uses the same exclusion set for all priority queues.
 */
std::pair <ushort, ushort> Dijkstra::best_pair(SkipList & pq){
  std::cout << "best pair" << std::endl;
  std::pair <ushort, ushort> curr_pair;
  do{
    curr_pair = pq.pop_uniform();
    std::cout << "pop = " << curr_pair.first << ", " << curr_pair.second << std::endl;
    //curr_pair = pq.pop();
  }while(!pq.empty() && (G1_exclude.find(std::get<0>(curr_pair)) != G1_exclude.end() ||
			 G2_exclude.find(std::get<1>(curr_pair)) != G2_exclude.end()) );
  std::cout << "picked seed (" << curr_pair.first << ", " << curr_pair.second << ")" << std::endl;
  return curr_pair;
}

/* 
 * side effects: this function adds a node to the respective exclusion sets
 * of each graph and adds the pair to the output set.  
 */
void Dijkstra::update_neighbors(std::pair <ushort, ushort> & seed_pair){
  //std::cout << "update neighbors" << std::endl;
  //exclude the seed nodes from future consideration
  //std::cout << "excluding nodes" << std::endl;
  G1_exclude.insert(std::get<0>(seed_pair));
  G2_exclude.insert(std::get<1>(seed_pair));
  //std::cout << "excluding nodes done" << std::endl;
  //add the pair to the alignment
  //std::cout << "output" << std::endl;
  //std::cout << std::get<0>(seed_pair) << "," << std::get<1>(seed_pair) << std::endl;
  A[std::get<0>(seed_pair)] = std::get<1>(seed_pair);
  nodes_aligned += 1;
  //std::cout << "output done" << std::endl;
  //set difference
  //std::cout << "set diff" << std::endl;
  vector<ushort> G1_neighbors = exclude_nodes(G1AdjLists[std::get<0>(seed_pair)], G1_exclude);
  vector<ushort> G2_neighbors = exclude_nodes(G2AdjLists[std::get<1>(seed_pair)], G2_exclude);
  //std::cout << "set diff done" << std::endl;
  //if there are no neighbors, we can skip the matrix search 
  if(G1_neighbors.empty() || G2_neighbors.empty()){
    return;
  }
	
  //add the possible neighbors to the set best_neighbors
  vector<std::pair<double, std::pair<ushort,ushort>>> new_neighbors = best_neighbors(G1_neighbors, G2_neighbors);
  //std::cout << "neighbors" << std::endl;
  for(unsigned int i = 0; i < new_neighbors.size(); ++i){
    double sim_value = std::get<0>(new_neighbors[i]);
    std::pair<ushort,ushort> node_pair = std::get<1>(new_neighbors[i]);
    //std::cout << sim_value << ": " << std::get<0>(node_pair) << "," << std::get<1>(node_pair) << std::endl;
    neighbor_queue.insert(sim_value, node_pair);
  }
}

vector<std::pair<double, std::pair<ushort,ushort>>> Dijkstra::best_neighbors(vector<ushort> & G1_neighbors, vector<ushort> & G2_neighbors){
  vector<std::pair<double, std::pair<ushort,ushort>>> out;
  vector<vector<double> > small_matrix (G1_neighbors.size(), vector<double> (G2_neighbors.size()));
	
  /*
    for(std::vector<ushort>::iterator G1_iter = G1_neighbors.begin(); G1_iter != G1_neighbors.end(); ++G1_iter){
    for(std::vector<ushort>::iterator G2_iter = G2_neighbors.begin(); G2_iter != G2_neighbors.end(); ++G2_iter){
    //make a similarity matrix of neighbors
    //small_matrix[*G1_iter][*G2_iter] = sims[*G1_iter][*G2_iter];
    }
    }
  */
  double max_sim = -1;
  for(unsigned int i = 0; i < G1_neighbors.size(); ++i){
    for(unsigned int j = 0; j < G2_neighbors.size(); ++j){
      small_matrix[i][j] = sims[G1_neighbors[i]] [G2_neighbors[j]];
      if(sims[G1_neighbors[i]] [G2_neighbors[j]] > max_sim){
	max_sim = sims[G1_neighbors[i]] [G2_neighbors[j]];
      }
    }
  }
	
  for(unsigned int i = 0; i < G1_neighbors.size(); ++i){
    for(unsigned int j = 0; j < G2_neighbors.size(); ++j){
      //insert pair into priority queue (skip list)
      if(small_matrix[i][j] >= (max_sim - delta)){
	out.push_back(std::make_pair(small_matrix[i][j], std::make_pair(G1_neighbors[i], G2_neighbors[j])));
      }
    }
  }
  return out;
}

/*
 * this simulates the set difference of an 
 * adjacency list and an exclusion set. 
 * It returns a vector containing the adjacent nodes 
 * which are not already aligned.  
 */
vector<ushort> Dijkstra::exclude_nodes(vector<ushort> & v_in, std::unordered_set<ushort> & exclusion_set){
  vector<ushort> v_out;
  v_out.reserve(v_in.size()); //minimize reallocations by reserving space in advance
  //|v_out| <= |v_in|
  for(std::vector<ushort>::iterator it = v_in.begin(); it != v_in.end(); ++it){
    if(exclusion_set.find(*it) == exclusion_set.end()){
      v_out.push_back(*it);
    }
  }
  return v_out;
}

/* End Helper Functions */

Alignment Dijkstra::run() {
  // Put all code here

  // sims[x][y] will give you how similar node x in G1 is to node y in G2 with 0 being the not at all similar

  // Put alignment into A[x] = y where x is an index of G1 and y is an index of G2

  //return Alignment(A);
  G1_exclude.clear();
  G2_exclude.clear();
  //current_neighbors.clear();
  //skiplist here
  make_seed_queue();
  //don't need these in the main body, only in the best pair routine
  //only make one copy

  std::cout << "Dijkstra begin: max_nodes= " << max_nodes << std::endl;
  while(nodes_aligned < max_nodes){
    std::cout << "seed phase" << std::endl;
    //seed phase
    std::pair <ushort, ushort> seed_pair = get_seed(G1,G2);
    update_neighbors(seed_pair);
	
    while(!neighbor_queue.empty()){
      std::cout << "extend phase" << std::endl;
      std::pair <ushort, ushort> neighbor_pair = best_pair(neighbor_queue);
      std::cout << "picked neighbor: (" << neighbor_pair.first << ", " << neighbor_pair.second << ")" << std::endl;
      update_neighbors(neighbor_pair);
    }
    std::cout << "nodes aligned" << nodes_aligned << std::endl;
  }
  
  outputA = Alignment(A);
  return outputA;
}


void Dijkstra::describeParameters(ostream& stream) {
  // Don't worry about this
}

string Dijkstra::fileNameSuffix(const Alignment& A) {
  return "_" + extractDecimals(MC->eval(A),3);
}

Dijkstra::~Dijkstra() {}

