#include "graphette.hpp"

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
        elems.push_back(item);
}

//For use in storing in canonical_graph
int convert(string s) {
    return stoi(s.substr(1,s.length()));
}

void canonical_array(int array[], int n) {
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
}

//Convert array node permutation to string
string s_permutation(int array[], int n) {
    string s = "";
    for (int i = 0; i < n; i++) {
        s += to_string(array[i]);
    }
    return s;
}

// This function takes a decimal number and a number of nodes
// to find the binary representation of the decimal number and
// store it in a vector. Length of vector depends on number of nodes.

// Ex:
// num_nodes = 4       --->    Length of vector must be 6
// decimal_number = 3  --->    011 in binary
// vector = [0, 1, 1]  --->    vector = [0, 0, 0, 0, 1, 1]
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

void generate_node_permutation_commands(int num_nodes, std::string file_name_variation)
{
    //Call split -l [x lines] [canonical_mapN_final.txt] [new_file names]
    //SAMPLE: split -l 100 canonical_map7_final.txt map
    ofstream executable("executable_permutations.txt");    
    
    char variation = 'a';
    while(true) {
        std::string filestring = file_name_variation;
        filestring+=variation;
        variation++;
        std::ifstream file(filestring);
        if (file.is_open()) {
            std::string args[] = {
                "./generate_permutations", 
                std::to_string(num_nodes), 
                filestring
            };
            executable << args[0] << " " << args[1] << " " << args[2] << "\n";
        }
        else {
            break;
        }
    }
}

//Generates the node permutations in parallel
//To execute use below command for parallelizing
// cat [executable.txt] | ~wayne/bin/bin.x86_64/parallel -s sh 32
void generate_node_permutations(int num_nodes, std::string file_name) 
{
    int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
    vector<bool> graph_bit_vector(num_possible_edges), current_canonical_bit_vector(num_possible_edges);
    
    std::ofstream fpermutation(file_name+"_permutation_map_"+to_string(num_nodes)+".txt");
    
    //Graphs used for final permutation fixing for the node orderings
    Graph* permutation_graph = new Graph(num_nodes);
    Graph* canonical_graph = new Graph(num_nodes);

    //Remapping all the non-canonicals that were mapped to the intermediate canonicals
    // by commutativity the true canonicals -> intermediate canonicals -> non-canonicals
    fpermutation << "graphette\tcanonical\tpermutation\r\n";
    int array[num_nodes];
    int canonical_node = 0;
    int decimal_representation = 0;
    
    std::vector<std::string> line;
    std::string canonical_to_noncanonical;
    std::ifstream file(file_name);
    
    if (file.is_open()) {
        while (getline(file, canonical_to_noncanonical, '\n')) {
            line.clear();
            split(canonical_to_noncanonical, ' ', line);
            //canonical node
            canonical_node = stoi(line[0]);
            //Resetting the canonical graph instead of recreating it to save Memory
            canonical_graph->reset();
            generate_bits_vector(canonical_node, current_canonical_bit_vector);
            canonical_graph->setAdjMatrix(current_canonical_bit_vector);
            
            for (int val = 0; val < line.size() - 1; val++) {
                decimal_representation = atoi(line[val].c_str());
                //The ismorphic code is a comparison from canonical to non-canonical form
                // restting the array to canonical 01234... form 
                // ensures that the comparison is always to canonical form
                canonical_array(array, num_nodes);
                
                if (decimal_representation != canonical_node) {        
                    //Loading the permutation/non-canonical graph 
                    permutation_graph->reset();
                    generate_bits_vector(decimal_representation, graph_bit_vector);
                    permutation_graph->setAdjMatrix(graph_bit_vector);
                    
                    //Generating the node permutations
                    graphIsomorphic(*canonical_graph, *permutation_graph, array);
                }
                //Writes to permutation_map.txt in 
                // Permutation Integer Representation        Canonical Node Integer Representation        Non-Canonical Node Permutation
                fpermutation << decimal_representation << "\t\t\t" << canonical_node << "\t\t\t" << s_permutation(array, num_nodes) << "\r\n";
            }
        }
        file.close();
    }
    
        //Clean-up
    delete permutation_graph;
    delete canonical_graph;
}

void generate_final_canonicals(int num_nodes, std::string file_name)
{
    std::vector<unsigned int> p_canonicals;
    std::map <unsigned int, std::map<unsigned int, std::string> > canonical_map;
    std::string p_canonical;
    std::vector<std::string> line;
    int current;
    int i = 0;
    //Filters the intermediate canonical permutation maps for the canonical to non-canonical mappings
    while(1) {
        current = -1;
        std::ifstream file(file_name+std::to_string(num_nodes)+"_"+std::to_string(i)+".txt");
        if (file.is_open()) {
            while (getline(file, p_canonical, '\n')) {
                line.clear();
                split(p_canonical, '\t', line);
                int current_compare = stoi(line[1]);
                int non_c = stoi(line[0]);
                if (current_compare != current ) {
                    current = current_compare;
                    p_canonicals.push_back(current_compare);
                }
                if (!canonical_map.count(current_compare)) {
                    std::map<unsigned int, std::string> permute;
                    permute[non_c] = line[2];
                    canonical_map[current_compare] = permute;
                }
                else {
                    canonical_map[current_compare][non_c] = line[2];
                }
            }
            file.close();
        }
        else {
            break;
        }
        ++i;
    }
    generate_final_canonicals_map(num_nodes, p_canonicals, canonical_map);
}

void generate_final_canonicals_map(int num_nodes, std::vector<unsigned int> &p_canonicals, std::map <unsigned int, std::map<unsigned int, string> > &canonical_map)
{
    //Holds the true/final canonicals for the node size         
    std::vector<string> graph_canonical;
    //Maps the true canonicals to the intermediate canonicals; element 0 of each vector is canonicals
    std::vector<std::vector<unsigned int>> mapping;
    
    graph_canonical = filter_intermediate_canonical(num_nodes, mapping, p_canonicals);
    std::cout << "Final canonicals generated...\n";

    ofstream fmap("canonical_map"+to_string(num_nodes)+"_"+"final"+".txt");

    std::vector<unsigned int> sorter;
    
    for (std::vector<unsigned int> s: mapping) {
        sorter.clear();
        for (unsigned int m: s) {
            for (std::pair<unsigned int, string> key_map: canonical_map[m]) {
                sorter.push_back(key_map.first);
            }
        }
        std::sort(sorter.begin(), sorter.end());
        for (unsigned int n: sorter) {
            fmap << n << " ";
        }
        fmap << "\r\n";
    }
}

//Generates a list of commands (located in executable.txt) that are used to generate the intermediate canonicals
void generate_intermediate_canonical_commands (int num_nodes, int BLOCK_SIZE, std::string file_name) 
{
    int INTERVAL = 10;
    
    int num_graphs = (num_nodes == 0) ? 0 : pow(2, (num_nodes*(num_nodes - 1)) / 2);
    int num_files = ceil(num_graphs/BLOCK_SIZE);
    ofstream executable("executable_intermediate.txt");
    int i = 0;
    std::cout << "Number of Graphs: " << num_graphs << " split into children of size " << BLOCK_SIZE << "\n";
    for (i; i < num_files; i+=INTERVAL) {
        char* args[] = {
            (char*)"./generate_intermediate_canonicals", 
            (char*)std::to_string(num_nodes).c_str(), 
            (char*)std::to_string(i).c_str(), 
            (char*)std::to_string(i+INTERVAL).c_str(), 
            (char*)file_name.c_str(),  
            NULL
        };
        executable << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << " " << args[4] << "\n";
    }
}

//Remaps intermediate canonicals to more intermediate canonicals 
void generate_intermediate_canonicals (int num_nodes, int start_file_interval, int end_file_interval, std::string file_name) 
{    
    std::vector<unsigned int> p_canonicals;
    std::map <unsigned int, std::map<unsigned int, std::string> > canonical_map;
    std::string p_canonical;
    std::vector<std::string> line;
    int current;
    //Filters the intermediate canonical permutation maps for the canonical to non-canonical mappings
    for (int i = start_file_interval; i < end_file_interval; ++i) {
        current = -1;
        std::ifstream file(file_name+std::to_string(num_nodes)+"_"+std::to_string(i)+".txt");
        if (file.is_open()) {
            while (getline(file, p_canonical, '\n')) {
                line.clear();
                split(p_canonical, '\t', line);
                int current_compare = stoi(line[1]);
                int non_c = stoi(line[0]);
                if (current_compare != current ) {
                    current = current_compare;
                    p_canonicals.push_back(current_compare);
                }
                if (!canonical_map.count(current_compare)) {
                    std::map<unsigned int, std::string> permute;
                    permute[non_c] = line[2];
                    canonical_map[current_compare] = permute;
                }
                else {
                    canonical_map[current_compare][non_c] = line[2];
                }
            }
            file.close();
        }
    }
    
    int variation = start_file_interval/(end_file_interval - start_file_interval);
    
    intermediate_canonicals_map(p_canonicals, canonical_map, num_nodes, variation);
}

//Goes through generating the intermediate canonicals and writes them to file
void intermediate_canonicals_map (std::vector<unsigned int> &p_canonicals,
    std::map <unsigned int, std::map<unsigned int, string> > &canonical_map, int num_nodes, int variation) {
            
    //Holds the true/final canonicals for the node size         
    std::vector<string> graph_canonical;
    //Maps the true canonicals to the intermediate canonicals; element 0 of each vector is canonicals
    std::vector<std::vector<unsigned int>> mapping;
    
    graph_canonical = filter_intermediate_canonical(num_nodes, mapping, p_canonicals);
    std::cout << "Intermediate canonicals generated...\n";

    ofstream fpermutation("intermediate_permutation_map"+to_string(num_nodes)+"_"+std::to_string(variation)+".txt");

    int canonical_node = 0;
    for (std::vector<unsigned int> s: mapping) {
        canonical_node = s[0];
        for (unsigned int m: s) {
            for (std::pair<unsigned int, string> key_map: canonical_map[m]) {
                fpermutation << key_map.first << "\t" << canonical_node << "\t" << "NOT_NEEDED" << "\n";
            }
        }
    }
}

//Remaps the intermediate canonicals to the true canonicals 
std::vector<string> filter_intermediate_canonical(int num_nodes, std::vector<std::vector<unsigned int>> &mapping, 
    std::vector<unsigned int> &p_canonicals) 
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
        if(i%1000 == 0)    std::cout << "Almost there..." << i*100.0/p_canonicals.size() << "%     \r\n";
        
        canonical_array(array, num_nodes);
        graph->reset();
        generate_bits_vector(p_canonicals[i], graph_bit_vector);
        graph->setAdjMatrix(graph_bit_vector);
        
        //Empty = first occurrence is obviously canonical
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
                //Check Isomorphism 
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
            //If its in canonical form its added to graph_canonical
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

//Generates a list of commands (located in executable.txt) that are used to generate the intermediate canonicals
void generate_intial_canonical_commands (int num_nodes, int BLOCK_SIZE) 
{
    int num_graphs = (num_nodes == 0) ? 0 : pow(2, (num_nodes*(num_nodes - 1)) / 2);
    
    ofstream executable("executable.txt");
    int i = 0;
    std::cout << "Number of Graphs: " << num_graphs << " split into children of size " << BLOCK_SIZE << "\n";
    for (i; i < num_graphs; i+= BLOCK_SIZE) {
        char* args[] = {
            (char*)"./graphette", 
            (char*)std::to_string(num_nodes).c_str(), 
            (char*)std::to_string(i/BLOCK_SIZE).c_str(), 
            (char*)std::to_string(BLOCK_SIZE).c_str(), 
            (char*)std::to_string(i).c_str(),  
            (char*)std::to_string(((i+BLOCK_SIZE < num_graphs) ? i+BLOCK_SIZE : num_graphs)).c_str(),
            NULL
        };
        executable << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << " " << args[4] << " " << args[5] << "\n";
    }
}

//Used for generating the initial/intermediate canonicals
void generate_initial_canonical (int num_nodes, int variation, int block_size, int start, int end) 
{        
    std::vector<string> graph_canonical;
    std::vector<std::vector<unsigned int>> mapping;
    std::vector<string> permutations(block_size);
    
    graph_canonical = generate_canonical(num_nodes, mapping, permutations, block_size, start, end);

    ofstream fpermutation("permutation_map"+to_string(num_nodes)+"_"+std::to_string(variation)+".txt");

    int canonical_node = 0;
    for (std::vector<unsigned int> s: mapping) {
        canonical_node = s[0];
        for (unsigned int m: s) {
            fpermutation << m << "\t" << canonical_node << "\t" << permutations[m - variation*block_size] << "\n";
        }
    }     
}

//Generates canonicals given start and end graph number 
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
            //If canonical then store
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
        // std::cout << s_permutation(preArray, n) << std::endl;
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