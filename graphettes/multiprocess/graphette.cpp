#include "graphette.hpp"

void final_canonicals (std::vector<unsigned int> &p_canonicals,
		std::map <unsigned int, std::map<unsigned int, string> > &canonical_map, int num_nodes) {
	int num_graphs = (num_nodes == 0) ? 0 : pow(2, (num_nodes*(num_nodes - 1)) / 2);
	std::vector<string> graph_canonical;
	std::vector<std::vector<unsigned int>> mapping;
	
	graph_canonical = generate_final_canonical(num_nodes, mapping, p_canonicals);

	ofstream fcanon("canonical_decimal_representation"+to_string(num_nodes)+".txt"), 
			fmap("canon_map"+to_string(num_nodes)+".txt"), fpermutation("permutation_map"+to_string(num_nodes)+".txt");
	
	// For writing to file
	fcanon << graph_canonical.size() << "\n";
	fpermutation << num_nodes << "\n";
	// This will show the decimal representations of adjMatrix of each Graph
	std::cout << "Canonical Graph adjMatrix dec_rep: ";
	for (string g : graph_canonical)
	{
		std::cout << "G" << g << " ";
		fcanon << g.substr(1,g.length()) << " ";
	}

	int array[num_nodes];
	int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
	vector<bool> graph_bit_vector(num_possible_edges), current_canonical_bit_vector(num_possible_edges);
	Graph* permutation_graph = new Graph(num_nodes);
	Graph* canonical = new Graph(num_nodes);
	
	fpermutation << "graphette\tcanonical\tpermutation\r\n";
	int canonical_node = 0;
	for (std::vector<unsigned int> s: mapping) {
		canonical_node = s[0];
		
		canonical->reset();
		generate_bits_vector(canonical_node, current_canonical_bit_vector);
		canonical->setAdjMatrix(current_canonical_bit_vector);
		
		for (unsigned int m: s) {
			for (std::pair<const unsigned int, string> permute: canonical_map[m]) {
				fmap << permute.first << " ";
				
				canonical_array(array, num_nodes);
		
				permutation_graph->reset();
				generate_bits_vector(permute.first, graph_bit_vector);
				permutation_graph->setAdjMatrix(graph_bit_vector);
				
				graphIsomorphic(*canonical, *permutation_graph, array);
				
				fpermutation << permute.first << "\t\t\t" << canonical_node << "\t\t\t" << s_permutation(array, num_nodes) << "\r\n";
			}
		}
		fmap << "\r\n";
	}
	delete permutation_graph;
	delete canonical;
	std::cout << "\n\n"; 
}

std::vector<string> generate_final_canonical(int num_nodes, std::vector<std::vector<unsigned int>> &mapping, std::vector<unsigned int> &p_canonicals) 
{
	int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
	vector<bool> graph_bit_vector(num_possible_edges), current_canonical_bit_vector(num_possible_edges);
	
	// replaced_Graph is used to determine whether to insert a new Graph into graph_canonical or not
	bool not_canonical_form = false;
	std::vector<string> graph_canonical;
	std::vector<string> copy_c;
		
	Graph* graph = new Graph(num_nodes);
	Graph* current_canonical = new Graph(num_nodes);
	int array[num_nodes];

	for (int i = 0; i < p_canonicals.size(); i++)
	{
		// Use iterator because the erase() function vector only accepts 
		// iterator as argument
		canonical_array(array, num_nodes);
		graph->reset();
		generate_bits_vector(p_canonicals[i], graph_bit_vector);
	    graph->setAdjMatrix(graph_bit_vector);
		
		//Empty = first is obviously canonical
		if (graph_canonical.empty())
		{
			graph_canonical.push_back("s"+to_string(p_canonicals[i]));
			std::vector<unsigned int> map;
			map.push_back(p_canonicals[i]);
			mapping.push_back(map);
		}
		else 
		{			
			for (auto j = copy_c.begin(); j != copy_c.end(); j++)
			{
				canonical_array(array, num_nodes);
				current_canonical->reset();
				generate_bits_vector(convert(*j), current_canonical_bit_vector);
				current_canonical->setAdjMatrix(current_canonical_bit_vector);
				if (graphIsomorphic(*current_canonical, *graph, array))
				{
					for (int k = 0; k < copy_c.size(); k++) {
						if (copy_c[k] == *j) {
							mapping[k].push_back(p_canonicals[i]);
							break;
						}
					}
					not_canonical_form = true;
					break;
				}
			}	
			if (!not_canonical_form) {
				graph_canonical.push_back("s"+to_string(p_canonicals[i]));
				std::vector<unsigned int> map;
				map.push_back(p_canonicals[i]);
				mapping.push_back(map);
			}
		}
		not_canonical_form = false;
		copy_c = graph_canonical;
	}

	delete graph;
	delete current_canonical;

	return graph_canonical;	
}

int convert(string s) {
	return stoi(s.substr(1,s.length()));
}

void canonical_array(int array[], int n) {
	for (int i = 0; i < n; i++) {
		array[i] = i;
	}
}

string s_permutation(int array[], int n) {
	string s = "";
	for (int i = 0; i < n; i++) {
		s += to_string(array[i]);
	}
	return s;
}

void generate_bits_vector(int decimal_number, vector<bool>& result)
{
	// Convert to binary number and put each bit in result vector.
	int i = result.size() - 1;
	for (int j = 0; j < i+1; j++) {
		result[j] = 0;
	}
	while (decimal_number != 0)
	{
		// This assertion assures that the length of binary in a decimal number
		// will not exceed the number of possible edges.
		// Ex: 
		// num_possible_edges = 6
		// Length of bits must be <= num_possible_edges
		assert(i >= 0 && "Binary length exceeds number of possible edges");
		result[i] = decimal_number % 2;
		--i;
		decimal_number /= 2;	
	}
}

std::vector<string> generate_canonical(int num_nodes, std::vector<std::vector<unsigned int>> &mapping, std::vector<string> &permutations, 
	int block_size, int start, int end) 
{
	int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
	vector<bool> graph_bit_vector(num_possible_edges), current_canonical_bit_vector(num_possible_edges);
	
	// replaced_Graph is used to determine whether to insert a new Graph into graph_canonical or not
	bool not_canonical_form = false;
	std::vector<string> graph_canonical;
	std::vector<string> copy_c;
		
	Graph* graph = new Graph(num_nodes);
	Graph* current_canonical = new Graph(num_nodes);
	int array[num_nodes];
	
	int array_count = 0;

	for (int i = start; i < end; i++)
	{
		// Use iterator because the erase() function vector only accepts 
		// iterator as argument
		canonical_array(array, num_nodes);
		graph->reset();
		generate_bits_vector(i, graph_bit_vector);
	    graph->setAdjMatrix(graph_bit_vector);
		
		if (graph_canonical.empty())
		{
			graph_canonical.push_back("s"+to_string(i));
			std::vector<unsigned int> map;
			map.push_back(i);
			mapping.push_back(map);
			permutations[array_count] = s_permutation(array, num_nodes);

		}
		else 
		{			
			for (auto j = copy_c.begin(); j != copy_c.end(); j++)
			{
				canonical_array(array, num_nodes);
				current_canonical->reset();
				generate_bits_vector(convert(*j), current_canonical_bit_vector);
				current_canonical->setAdjMatrix(current_canonical_bit_vector);
				if (graphIsomorphic(*current_canonical, *graph, array))
				{
					for (int k = 0; k < copy_c.size(); k++) {
						if (copy_c[k] == *j) {
							permutations[array_count] = s_permutation(array, num_nodes);
							mapping[k].push_back(i);
							break;
						}
					}
					not_canonical_form = true;
					break;
				}
			}	
			if (!not_canonical_form) {
				graph_canonical.push_back("s"+to_string(i));
				std::vector<unsigned int> map;
				map.push_back(i);
				mapping.push_back(map);
				permutations[array_count] = s_permutation(array, num_nodes);
			}
		}
		not_canonical_form = false;
		copy_c = graph_canonical;
		array_count++;
	}

	delete graph;
	delete current_canonical;

	return graph_canonical;	
}

vector<vector<bool>> decimal_to_matrix(int decimal_number, int num_nodes)
{
	int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
	vector<bool> bv(num_possible_edges);
	generate_bits_vector(decimal_number, bv);
	vector<vector<bool>> result(num_nodes, vector<bool>(num_nodes));

	int k = 0;
	for (int i = 0; i < num_nodes; i++)
	{
		for (int j = 0; j < num_nodes; j++)
		{
			if (i < j)
			{
				result[i][j] = bv[k];
				k++;
			}
		}
	}
	
	return result;
}


using namespace std;
static Graph *isoG1, *isoG2;
//Used for isomorphism checking
bool GraphAreConnected(Graph *G, int i, int j)
{
    assert(0 <= i && i < (int)G->getNumNodes() && 0 <= j && j < (int)G->getNumNodes());

	int k, n, me, other;
	vector<ushort> neighbors;
	// Check through the shorter list
	if(G->getDegree(i) < G->getDegree(j))
	{
	    me = i; other = j;
	}
	else
	{
	    me = j; other = i;
	}
	n = G->getDegree(me);
	//Convert this to SANA graph class neighbors = G->returnAdjLists()[me];
	vector<vector<ushort>> adjList;
	G->getAdjLists(adjList);
	neighbors = adjList[me];
	for(k=0; k<n; k++)
	    if(neighbors[k] == other)
		return true;
	return false;

}

static bool _allPermutations(int n, int i, int *preArray, bool (*fcn)(int, int *))
{
    int j;
    if( n == i) {  /* output! */
		std::cout << s_permutation(preArray, n) << std::endl;
		return fcn(n, preArray);
	}
    for(j=0; j < n; j++)    /* put in slot i all j's not already appearing */
    {
		int k;
		for(k=0; k<i; k++)  /* see if this j's already been used */
		{
			if(preArray[k] == j)
			break;
		}
		if(k == i)  /* this j hasn't appeared yet */
		{
			int result;
			preArray[i] = j;
			if((result = _allPermutations(n, i+1, preArray, fcn)))
			return result;
		}
    }
    return false;
}

bool _permutationIdentical(int n, int perm[])
{
    int i, j;
    for(i=0; i<n; i++)
	if(isoG1->getDegree(i) != isoG2->getDegree(perm[i]))
	    return false;

    for(i=0; i<n; i++) for(j=i+1; j<n; j++)
	/* The !GraphAreConnected is just to turn a bitstring into a boolean */
	if(!GraphAreConnected(isoG1, i,j) !=
	    !GraphAreConnected(isoG2, perm[i], perm[j]))
	    return false;   /* non-isomorphic */
    return true;   /* isomorphic! */
}

bool CombinAllPermutations(int n, bool (*fcn)(int, int *), int array[])
{

    return _allPermutations(n, 0, array, fcn);
}

bool graphIsomorphic(Graph& G1, Graph& G2, int array[])
{
    int i, n = G1.getNumNodes(), degreeCount1[n], degreeCount2[n];
	
    if(G1.getNumNodes() != G2.getNumNodes())
    	return false;

    if(n < 2)
    	return true;

    for(i=0; i<n; i++)
	degreeCount1[i] = degreeCount2[i] = 0;

    for(i=0; i<n; i++)
    {
	   ++degreeCount1[G1.getDegree(i)];
	   ++degreeCount2[G2.getDegree(i)];
    }

    for(i=0; i<n; i++)
	   if(degreeCount1[i] != degreeCount2[i])
	       return false;
    isoG1 = &G1; isoG2 = &G2;
    return !!CombinAllPermutations(n,_permutationIdentical, array);
	
}

void print_matrix(const std::vector<std::vector<bool>>& matrix)
{
	for (unsigned int i = 0; i < matrix.size(); i++)
	{
		for (unsigned int j = 0; j < matrix.size(); j++)
		{
			if (i < j)
				std::cout << matrix[i][j] << " ";
			else
				std::cout << "  ";
		}
		std::cout << "\n";
	}
}

bool contain_edge(std::vector<ushort>& edge, std::vector<std::vector<ushort>>& edge_vector)
{
	for (const std::vector<ushort>& e : edge_vector)
	{
		if (edge == e)
			return true;
	}
	return false;
}

std::vector<std::vector<ushort>> remove_duplicate_edges(Graph& g)
{
	std::vector<std::vector<ushort>> copy_adjList;
	g.getAdjLists(copy_adjList);
	std::vector<ushort> edge, reverse_edge;
	std::vector<std::vector<ushort>> edge_vector;

	for (unsigned int i = 0; i < copy_adjList.size(); i++)
    {
    	// Check if a node has an edge to itself
    	if (copy_adjList[i].empty())
    	{
    		edge = {static_cast<ushort>(i),static_cast<ushort>(i)};
    		edge_vector.push_back(edge);
    		continue;
    	}
        for (unsigned int j = 0; j < copy_adjList[i].size(); j++)
        {
        	edge = {static_cast<ushort>(i), copy_adjList[i][j]};
        	reverse_edge = {copy_adjList[i][j], static_cast<ushort>(i)};
        	if (!contain_edge(edge, edge_vector) && !contain_edge(reverse_edge, edge_vector))
        	{
        		edge_vector.push_back(edge);
        	}
        }
    }

    return edge_vector;
}

std::vector<ushort> assign_node_position(int n)
{
	ushort x, y;
	std::vector<ushort> result;

	switch (n)
	{
		case 0:
			x = 0; y = 0;
			break;
		case 1:
			x = 0; y = 2;
			break;
		case 2:
			x = 0; y = 4;
			break;
		case 3:
			x = 4; y = 0;
			break;
		case 4:
			x = 4; y = 2;
			break;
		case 5:
			x = 4; y = 4;
			break;
	}
	result.push_back(x);
	result.push_back(y);

	return result;
}

void write_graphs_to_file(std::string file, std::vector<std::vector<ushort>> adjList, int num_nodes)
{
	std::ofstream output(file);

	// output << "graph Graphette\n{\n\trankdir = LR;\n";
	output << "graph Graphette\n{\n";

	for (int i = 0; i < num_nodes; i++)
	{
		int x = assign_node_position(i)[0];
		int y = assign_node_position(i)[1];

		output << "\t" << i << " [pos = \"" << x << ", " << y << "!\"]\n";

	}

	for (const std::vector<ushort>& i : adjList)
	{
		if (i[0] == i[1])
			output << "\t" << i[0] << ";\n";
		else
			output << "\t" << i[0] << " -- " << i[1] << ";\n";
	}

	output << "}";
	output.close();
}