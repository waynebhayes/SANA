#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <cassert>
=======


std::string input_error_prompt = "currently the expected input is:\nFor transformation: graph_morph -t transition_number(integer) edgelist1(file path) edgelist2(file path) output_directory(file path directory)\nFor combination: graph_morph -c weight1(float). . . weightn(float) edgelist1(file path) . . .  edgelist2(file path) output_directory(file path directory)\ndirectory should be a path with a '\\' or '/' depending on system\nEdge list files should have rows of two integers\n";
enum morph_mode {transformation, combination};

typedef std::pair<int, int> edge;

struct edge_hash{

	public:

	std::size_t operator()(const std::pair<int, int> & e) const
	{
		return std::hash<int>()(e.first) ^ std::hash<int>()(e.second);
	}

};


typedef std::unordered_set<edge,  edge_hash> graph;

void edges_from_file(graph & graph_edges, std::ifstream & filename)
{
	int node1, node2;
	while (filename >> node1 >> node2)
	{
		
		graph_edges.insert(edge(std::min(node1, node2), std::max(node1, node2)));
	}

}

void edges_to_file(std::string name, graph & g)
{
	std::ofstream fil(name);
	for(graph::iterator e = g.begin(); e != g.end(); ++e)
		fil << e->first << " " << e->second << '\n';
	fil.close();	
}		
void print_graph(graph & g)
{
	for(graph::iterator e = g.begin(); e != g.end(); ++e)
		std::cout << e->first << " " << e->second << '\n';
}



float prob()
{
	return rand() / ((float) (RAND_MAX + 1.0));
}

int prob_transform(int alpha, int total)
{
	return prob() <= (float) 1/ (float) (total - (alpha - 1));
}



void difference_update(graph & a, graph & b)
{
	for(graph::iterator it = a.begin(); it != a.end();)
	{
		if(b.find(*it) != b.end() || b.find(edge(it->second, it->first)) != b.end())
			it = a.erase(it);
		else
			++it;

	}
}
void transform_step(graph & curg, graph & baseg, graph & endg, int alpha, int total)
{
	
	for(graph::iterator it = curg.begin(); it != curg.end();)
	{
		if(prob_transform(alpha, total))
		{
			baseg.erase(*it);
			it = curg.erase(it);
		}
		else
			++it;
	}
	
	for(graph::iterator it = endg.begin(); it != endg.end();)
	{
		if(prob_transform(alpha, total))
		{
			baseg.insert(*it);
			it = endg.erase(it);
		}
		else
			++it;

	}

	
}


void transform_full(std::string name, graph & g1, graph & g2, int saved_transitions, int total)
{
	graph g1_base(g1);
	difference_update(g1, g2);
	difference_update(g2, g1_base);
	
	

	
	for(int transition = 0; transition <= total + 1; ++transition)
	{
		transform_step(g1,g1_base, g2, transition, total);
		if(transition % saved_transitions == 0)
		
			edges_to_file(name + std::string("_") + std::to_string(transition), g1_base); 	
		
		
	}
		

	
	
}

void combo_multi(std::string name, graph graphs[], float weights[], graph & output, int sz)
{
	std::unordered_map<edge, float, edge_hash> edgeFreq;
	for(int i = 0; i < sz; ++i)
	{
		graph g_i = graphs[i];
		float w_i = weights[i];
		for(graph::iterator it = g_i.begin(); it != g_i.end(); ++it)
		{
			edgeFreq[*it] += w_i;
		}
	}
	for(std::unordered_map<edge, float, edge_hash>::iterator it = edgeFreq.begin(); it != edgeFreq.end(); ++it)
	{
		
		if(prob() <= (*it).second)
			output.insert((*it).first);

	}
	

	for(int i = 0; i < sz; ++i)
		name +=   std::to_string(weights[i]) + std::string("_");
	edges_to_file(name, output); 
	
			 	
}

void permute(graph & original, graph & to_permute)
{
	std::unordered_map<int, int> permute_map;
	std::vector<int> unique_nodes;

	for(graph::iterator e = original.begin(); e != original.end(); ++e)
	{
		if(permute_map.find(e->first) == permute_map.end())
		{
			permute_map[e->first];
			unique_nodes.push_back(e->first);
		}

		if(permute_map.find(e->second) == permute_map.end())
		{
			permute_map[e->second];
			unique_nodes.push_back(e->second);
		}
		
	}

	std::random_shuffle(unique_nodes.begin(), unique_nodes.end());
	
	
	for(auto pair = permute_map.begin(); pair != permute_map.end(); ++pair){
		permute_map[pair->first] = unique_nodes.back();
		unique_nodes.pop_back();
		//std:: cout << pair->first << " " << permute_map[pair->first] << std::endl;
	}	
	
	for(graph::iterator e = original.begin(); e != original.end(); ++e)
	{

		//std::cout << e->first << ":" << permute_map[e->first] << " " << e->second << ":" << permute_map[e->second] << std::endl;
		to_permute.insert(edge(permute_map[e->first], permute_map[e->second]));
	}
	
	assert(original.size() == to_permute.size());


}
morph_mode parse_args(int args, char * param[])
{
	morph_mode to_return;
	if (strcmp(param[1],"-t") == 0)
		to_return = transformation;
	else if(strcmp(param[1], "-c") == 0)
		to_return = combination;
	else
		throw std::invalid_argument(input_error_prompt);
		
	if(args < 6 )
		throw std::invalid_argument(input_error_prompt);
	
	std::ifstream isDir(param[args - 1]);
	if(!isDir)
		throw std::invalid_argument(std::string("Invalid directory, check arguments:\n") + input_error_prompt);

	
	return to_return;

}

int main(int argc, char *argv[])
{

	
	std::string rand_opt("*rand");

	morph_mode m = parse_args(argc, argv);
	std::string dump_folder = argv[argc - 1];
	if(m == transformation)
	{
		std::string f_name1 = argv[3];
		std::string f_name2 = argv[4];
		int transitions = std::atoi(argv[2]);
		std::ifstream f1;
		std::ifstream f2;
		graph g1;
		graph g2;
		try{
			f1.open(f_name1);
			edges_from_file(g1, f1);
			if(f_name2 == rand_opt)
				permute(g1, g2);
			else
			{
				f2.open(f_name2);
				edges_from_file(g2, f2);
			}
		}
		catch (...){
		
			throw std::invalid_argument("Issue opening and parsing files, ensure edgelist files consist of rows of two integers");
		}
		f1.close();
		if(f_name2 != rand_opt)
			f2.close();
		else
			f_name2 = std::string("rand");
		transform_full(dump_folder  + f_name1 + std::string("_to_") +  f_name2 , g1, g2, 1, transitions);
	}
	else if(m == combination)
	{
		int sz = (argc - 3) / 2;
		graph graphs[sz];
		float weights[sz];
		for(int i = 2; i < 2 + sz; ++i)
		{
			weights[i - 2] = std::atof(argv[i]);
			std::ifstream f;

			try
			{
				
				f.open(argv[sz + i]);
				edges_from_file(graphs[i - 2], f);
			}
			catch (...)
			{
				throw std::invalid_argument("Issue opening and parsing files, ensure edgelist files consist of rows of two integers");
			}
			f.close();
			
			
		}
			
		graph output;
		combo_multi(dump_folder, graphs, weights, output, sz); 

	}
	return 0;

}


