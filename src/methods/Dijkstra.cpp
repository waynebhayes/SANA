#include "Dijkstra.hpp"
#include "../utils/utils.hpp"

Dijkstra::Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC) :
			Method(G1, G2, "Dijkstra_"+MC->toString()) {

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
	startA = Alignment.empty();
	
	sims = MC->getAggregatedLocalSims();
	//sim_matrix = & sims;
}

/* Helper functions */

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
std::pair <ushort, ushort> Djikstra::get_seed(Graph* G1, Graph* G2){
	//only one seed at a time
	return best_pair(seed_queue);
	/*
	std::pair <ushort, ushort> seed_pair;
	do{
		seed_pair = std::make_pair(0,0);//best seed pair subroutine
	while(G1_exclude.find(std::get<0>(seed_pair)) != G1_exclude.end() ||
		G2_exclude.find(std::get<1>(seed_pair)) != G2_exclude.end() );
	*/
}

/*
 * This function uses the same exclusion set for all priority queues.
 */
std::pair <ushort, ushort> Djikstra::best_pair(SkipList * pq){
	std::pair <ushort, ushort> curr_pair;
	do{
		curr_pair = std::make_pair(0,0);
		//curr_pair = pq.pop();
	while(G1_exclude.find(std::get<0>(curr_pair)) != G1_exclude.end() ||
		G2_exclude.find(std::get<1>(curr_pair)) != G2_exclude.end() );
	return curr_pair;
}

/* 
 * side effects: this function adds a node to the respective exclusion sets
 * of each graph and adds the pair to the output set.  
*/
void Djikstra::update_neighbors(std::pair <ushort, ushort> & seed_pair,
	vector<ushort> & seed1_adj, vector<ushort> & seed2_adj ){
	//exclude the seed nodes from future consideration
	G1_exclude.insert(std::get<0>(seed_pair));
	G2_exclude.insert(std::get<1>(seed_pair));
	(*outputA)[std::get<0>(seed_pair)] = std::get<1>(seed_pair);
	
	//set difference
	vector<ushort> G1_neighbors = exclude_nodes(seed1_adj, G1_exclude);
	vector<ushort> G2_neighbors = exclude_nodes(seed2_adj, G2_exclude);
	
	//if there are no neighbors, we can skip the matrix search 
	if(G1_neighbors.empty() || G2_neighbors.empty()){
		return;
	}
	
	//add the possible neighbors to the set best_neighbors
	vector<std::pair<double, std::pair<ushort,ushort>>> new_neighbors = best_neighbors(G1_neighbors, G2_neighbors);
	for(int i = 0; i < new_neighbors.size(); ++i){
		double sim_value = std::get<0>(new_neighbors[i]);
		std::pair<ushort,ushort> node_pair = std::get<1>(new_neighbors[i]);
		//neighbor_queue.enqueue(sim_value, node_pair)
	}
}

vector<std::pair<double, std::pair<ushort,ushort>>> Djikstra::best_neighbors(vector<ushort> & G1_neighbors, vector<ushort> & G2_neighbors){
	double temp_delta = 0;
	vector std::pair<ushort,ushort> out;
	vector<vector<double> > small_matrix (G1_neighbors->size(), vector<double> (G2_neighbors->size()));
	
	/*
	for(std::vector<ushort>::iterator G1_iter = G1_neighbors.begin(); G1_iter != G1_neighbors.end(); ++G1_iter){
		for(std::vector<ushort>::iterator G2_iter = G2_neighbors.begin(); G2_iter != G2_neighbors.end(); ++G2_iter){
		//make a similarity matrix of neighbors
		//small_matrix[*G1_iter][*G2_iter] = sim_matrix[*G1_iter][*G2_iter];
		}
	}
	*/
	double max_sim = -1;
	for(int i = 0; i < G1_neighbors.size(); ++i){
		for(int j = 0; j < G2_neighbors.size(); ++j){
			small_matrix[i][j] = sim_matrix[G1_neighbors[i]] [G2_neighbors[j]];
			if(sim_matrix[G1_neighbors[i]] [G2_neighbors[j]] > max_sim){
			max_sim = sim_matrix[G1_neighbors[i]] [G2_neighbors[j]];
			}
		}
	}
	
	for(int i = 0; i < G1_neighbors.size(); ++i){
		for(int j = 0; j < G2_neighbors.size(); ++j){
		//insert pair into priority queue (skip list)
		if(small_matrix[i][j] >= (max_sim - temp_delta)){
			out.push_back(std::make_pair(small_matrix[i][j], std::make_pair(G1_neighbors[i], G2_neighbors[j])));
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
vector<ushort> exclude_nodes(vector<ushort> & v_in, std::unordered_set<ushort> exclusion_set){
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
	current_neighbors.clear();
	//skiplist here
	
	//don't need these in the main body, only in the best pair routine
	//only make one copy
	vector<vector<ushort> > G1AdjLists;
	G1->getAdjLists(G1AdjLists);
	vector<vector<ushort> > G2AdjLists;
	G2->getAdjLists(G2AdjLists);

	while(outputA->size() < max_nodes){
		//seed phase
		std::pair <ushort, ushort> seed_pair = get_seed(G1,G2);
		vector<ushort> & G1_neighbors = G1AdjLists[std::get<0>(seed_pair)];
		vector<ushort> & G2_neighbors = G1AdjLists[std::get<1>(seed_pair)];	
		update_neighbors(seed_pair, G1_neighbors, G2_neighbors);
	
		while(false){//!neighbor_queue.empty()
			std::pair <ushort, ushort> neighbor_pair = best_pair(neighbor_queue);
			G1_neighbors = G1AdjLists[std::get<0>(neighbor_pair)];
			G2_neighbors = G1AdjLists[std::get<1>(neighbor_pair)];	
			update_neighbors(neighbor_pair, G1_neighbors, G2_neighbors);
		}
	}

	return outputA;
}


void Dijkstra::describeParameters(ostream& stream) {
	// Don't worry about this
}

string Dijkstra::fileNameSuffix(const Alignment& A) {
	return "_" + extractDecimals(MC->eval(A),3);
}

Dijkstra::~Dijkstra() {}

