#include "graphette.hpp"

std::vector<bool> bits_vector(int decimal_number, int num_nodes)
{
    int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
    std::vector<bool> result(num_possible_edges);

    // Convert to binary number and put each bit in result vector.
    int i = result.size() - 1;
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

    return result;
}


std::vector<std::vector<bool>> generate_all_bits_vectors(int num_nodes)
{
    int num_vectors = pow(2, (num_nodes*(num_nodes - 1)) / 2);
    std::vector<std::vector<bool>> result;
    
    for (int i = 0; i < num_vectors; i++)
    {
        // Use i to make bits vector
        //std::vector<bool> bv = bits_vector(i, num_nodes);
        //result[i] = bv;
        result.push_back(bits_vector(i, num_nodes));
    }    

    return result;
}


std::vector<Graph*> generate_all_graphs(int num_nodes)
{
    std::vector<std::vector<bool>> all_bits_vectors = generate_all_bits_vectors(num_nodes);
    int num_graphs = pow(2, (num_nodes*(num_nodes - 1)) / 2);
    std::vector<Graph*> result;

    for (int i = 0; i < num_graphs; i++)
    {
        // The way I have constructed these Graphs is to define a new 
        // constructor in the Graph class. This new constructor takes in 
        // a number of nodes, and initializes its adjMatrix and adjList vectors.
        // Otherwise, we will get Segmentation Fault if we don't intialize these
        // private members in Graph class.
        Graph* g = new Graph(num_nodes);
        g->setAdjMatrix(all_bits_vectors[i]);
        g->set_decimal_representation(i);
        result.push_back(g);
        if(i%10000==0) cout << "Generating: " << i << "/" << num_graphs << " (" << i*100.0/num_graphs << "%)                 \r";
    }
    cout << endl;

    return result;
}


std::vector<Graph*> generate_canonical(const std::vector<Graph*>& graph_vectors)
{
    // replaced_Graph is used to determine whether to insert a new Graph into graph_canonical or not
    bool replaced_Graph = false;
    std::vector<Graph*> graph_canonical;
    std::vector<Graph*> copy_c;
    long long int num_graphs=graph_vectors.size(), currGraph=0;

    for (auto i : graph_vectors)
    {
        if(currGraph%1000==0) cout << "Sifting for canonicals: " << currGraph << "/" << num_graphs << " (" << currGraph*100.0/num_graphs << "%)                 \r";
        ++currGraph;
        if (graph_canonical.empty())
        {
            graph_canonical.push_back(i);
        }

        else
        {
            for (auto j = copy_c.begin(); j != copy_c.end(); j++)
            {
                if (graphIsomorphic(**j, *i))
                {
                    replaced_Graph = true;
                    if ((*j)->get_decimal_representation() > i->get_decimal_representation())
                    {
                        graph_canonical.erase(j);
                        graph_canonical.push_back(i);
                    }
                    break;
                }
            }

            if (!replaced_Graph)
                    graph_canonical.push_back(i);
        }

        replaced_Graph = false;
        copy_c = graph_canonical;
    }    
    cout << endl;

    return graph_canonical;    
}


Graph* get_canonical(Graph* g, std::vector<Graph*> graph_canonical)
{
    for (auto i : graph_canonical)
    {
        if (graphIsomorphic(*i, *g))
        {
            return i;
        }
    }    

    return nullptr;
}


using namespace std;
static Graph *isoG1, *isoG2;

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

static bool _allPermutations
    (int n, int i, int *preArray, bool (*fcn)(int, int *))
{
    int j;
    if( n == i) /* output! */
    return fcn(n, preArray);

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

bool CombinAllPermutations(int n, bool (*fcn)(int, int *))
{
    int array[n];
    return _allPermutations(n, 0, array, fcn);
}


bool graphIsomorphic(Graph& G1, Graph& G2)
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
    return !!CombinAllPermutations(n,_permutationIdentical);

}

vector<ushort> canonicalMapping(vector<Graph*>& graph_vectors, vector<Graph*>& graph_canonical)
{
    vector<ushort> map(graph_vectors.size());
    for (Graph* g: graph_vectors)
    {
        for(Graph* cg: graph_canonical)
        {
            if(graphIsomorphic(*g,*cg))
            {
                map[g->get_decimal_representation()] = cg->get_decimal_representation();
                break;
            }

        }
    }
    return map;
}


std::vector<std::vector<bool>> decimal_to_matrix(int decimal_number, int num_nodes)
{
    std::vector<bool> bv = bits_vector(decimal_number, num_nodes);
    std::vector<std::vector<bool>> result(num_nodes, std::vector<bool>(num_nodes));

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


void print_matrix(std::vector<std::vector<bool>> matrix)
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
    for (auto e : edge_vector)
    {
        if (edge == e)
            return true;
    }
    return false;
}


Graph random_Graph(int num_nodes)
{
    if (num_nodes == 0 || num_nodes == 1)
    {
        return Graph(num_nodes);
    }

    srand((int) time(0));
    int random_number_of_edges = rand() % (num_nodes*(num_nodes-1)) / 2;

    std::vector<std::vector<ushort>> random_matrix;
    std::vector<std::vector<ushort>> edge_vector;
    ushort random_node1, random_node2;
    std::vector<ushort> edge, reverse_edge;

    for (int i = 0; i < random_number_of_edges; i++)
    {
        random_node1 = rand() % num_nodes;
        random_node2 = rand() % num_nodes;

        // This prevents a node to have an edge to itself
        while (random_node1 == random_node2)
        {
            random_node1 = rand() % num_nodes;
            random_node2 = rand() % num_nodes;
        }
        edge = {random_node1, random_node2};
        reverse_edge = {random_node2, random_node1};

        // This prevents a node to have an edge to itself and only generates undirected edge        
        if (contain_edge(edge, edge_vector) || contain_edge(reverse_edge, edge_vector))
        {
            while (random_node1 == random_node2 || contain_edge(edge, edge_vector) || contain_edge(reverse_edge, edge_vector))
            {
                random_node1 = rand() % num_nodes;
                random_node2 = rand() % num_nodes;
                edge = {random_node1, random_node2};
                reverse_edge = {random_node2, random_node1};
            }
        }

        random_matrix.push_back(std::vector<ushort>{random_node1, random_node2});
        edge_vector.push_back(edge);
    }

    //// Debugging Purpose:
    // std::cout << "Max # of edges: " << (num_nodes*(num_nodes-1)) / 2 << "\n";
    // std::cout << "Number of edges: " << random_number_of_edges << "\n";
    // std::cout << "Size of Matrix: " << random_matrix.size() << "\n\n";

    return Graph(num_nodes, random_matrix);
}










