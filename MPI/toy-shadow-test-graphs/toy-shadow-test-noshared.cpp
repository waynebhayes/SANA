// run the following commands on odin to import MPI and increase memory limit
// module load mpi/openmpi3-x86_64
// ulimit -l 65536

// mpirun -np (number of processes) toy-shadow-test
// ex:
// mpirun -np 5 toy-shadow-test-noshared ../../regression-tests/shadow/syeast25.el ../../regression-tests/shadow/syeast20.el ../../regression-tests/shadow/syeast15.el ../../regression-tests/shadow/syeast10.el

#include <mpi.h>
#include <string>
#include <vector>
#include <array>

// SANA
#include "../../src/Alignment.hpp"
#include "../../src/Graph.hpp"
#include "../../src/arguments/GraphLoader.hpp"

using namespace std;

const int REMOVE_EDGE = 0;
const int ADD_EDGE = 1;
const int FINISHED = 2;

void addEdge(int node1, int node2)
{
    int edge[] = {node1, node2};
    // cout << "sending: " << node1 << " " << node2 << endl;
    MPI_Send(edge, 2, MPI_INT, 0, ADD_EDGE, MPI_COMM_WORLD);
}

void sendFinished()
{
    int x = 0;
    cout << "sending: finished" << endl;
    MPI_Send(&x, 1, MPI_INT, 0, FINISHED, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
    // Initial
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0 && size < argc)
    {
        printf("This program needs to be run with at least %d processes (networks + 1).\n", argc);
        MPI_Finalize();
        return 0;
    }
    // thread 0 loads graphs
    int numShadNodes = 0;

    // G2 is adjacency matrix / shadow
    if (rank == 0)
    {
        vector<Graph> graphs;
        for (int i = 1; i < argc; i++)
        {
            // avoid push_back
            graphs.push_back(GraphLoader::loadGraphFromFile(argv[i], argv[i], false));
        }
        int previous = -1;
        int largest = -1;
        for (Graph g : graphs)
        {
            int numNodes = g.getNumNodes();
            if (previous < 0)
            {
                // largest should be first graph
                previous = numNodes;
                largest = numNodes;
            }
            else
            {
                assert(numNodes <= previous);
            }

            cout << g.getName() << " numNodes: " << g.getNumNodes() << endl;
        }
        numShadNodes = largest * 1.1;

        MPI_Barrier(MPI_COMM_WORLD);
        // share numShadowNodes
        for (int i = 1; i < size; i++)
        {
            MPI_Send(&numShadNodes, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        printf("Broadcasted numShadowNodes\n");
        MPI_Barrier(MPI_COMM_WORLD);

        // receive edges
        // edges and weight from smaller->larger node
        vector<unordered_map<uint, int>> shadNbrSets(numShadNodes);

        int finished = 0;
        while (finished < size - 1)
        {
            int edge[] = {-1, -1};
            MPI_Status status;
            // cout << "receiving " << endl;
            MPI_Recv(edge, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // cout << "received: ";
            if (status.MPI_TAG == ADD_EDGE)
            {
                // cout << edge[0] << " - " << edge[1] << endl;
                shadNbrSets[edge[0]][edge[1]]++;
            }
            else if (status.MPI_TAG == REMOVE_EDGE)
            {
                cout << "remove";
            }
            else
            {
                finished++;
                cout << "finish" << endl;
            }
        }

        // from CreateShadow.cpp
        // check if continuous or not else we need to use low level array
        vector<array<uint, 2>> shadEdgeList;
        vector<int> shadEdgeWeights;

        uint numShadEdges = 0;
        for (const auto &nbrSet : shadNbrSets)
            numShadEdges += nbrSet.size();
        shadEdgeList.reserve(numShadEdges);
        shadEdgeWeights.reserve(numShadEdges);
        for (uint node = 0; node < numShadNodes; node++)
        {
            for (const auto &kv : shadNbrSets[node])
            {
                uint nbr = kv.first;
                int weight = kv.second;
                shadEdgeList.push_back({node, nbr});
                shadEdgeWeights.push_back(weight);
            }
        }
        for (int i = 0; i < shadEdgeList.size(); i++)
        {
            printf("(%d, %d) weight: %d\n", shadEdgeList[i][0], shadEdgeList[i][1], shadEdgeWeights[i]);
        }
    }
    else
    {
        // // worker threads load their local network and set shadow node size
        Graph G1 = GraphLoader::loadGraphFromFile(argv[rank], argv[rank], false);
        printf("[RANK %d] G1 %d nodes,  %d edges\n", rank, G1.getNumNodes(), G1.getNumEdges());

        MPI_Barrier(MPI_COMM_WORLD);
        // share numShadowNodes
        MPI_Recv(&numShadNodes, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[RANK %d] set numShadowNodes to %d\n", rank, numShadNodes);
        MPI_Barrier(MPI_COMM_WORLD);

        // Create initial alignment
        // map identity for now
        int n1 = G1.getNumNodes();
        int A[n1];
        for (int i = 0; i < n1; i++)
        {
            A[i] = i;
        }
        // Alignment A = Alignment::randomColorRestrictedAlignment(shadow, G1);
        for (const auto &edge : *(G1.getEdgeList()))
        {
            // send addEdge(a, b) with first parameter being smaller
            if (A[edge[0]] < A[edge[1]])
            {
                addEdge(A[edge[0]], A[edge[1]]);
            }
            else
            {
                addEdge(A[edge[1]], A[edge[0]]);
            }
        }
        sendFinished();
    }

    // Finalisation
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("FINAL BARRIER -> Finalize.\n");
    }

    MPI_Finalize();
}

// mpirun -np 11 main 1> output.txt