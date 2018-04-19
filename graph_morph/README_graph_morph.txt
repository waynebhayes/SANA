Placeholder README for graph_morph.cpp

Make sure to first do:

$g++ -std=c++11 graph_morph.cpp -o graph_morph

currently the expected input is:
	For transformation: graph_morph -t transition_number(integer) edgelist1(file path) edgelist2(file path) output_directory(file path directory)
	For combination: graph_morph -c weight1(float). . . weightn(float) edgelist1(file path) . . .  edgelist2(file path) output_directory(file path directory)
	directory should be a path with a '\\' or '/' depending on system\nEdge list files should have rows of two integers\n"


There is also now an option to do a random permutation for -t by writing *rand as a file argument:

graph_morph -t transition_number(integer) edgelist(file path) *rand output_directory(file path directory)

Creates a transformation from the graph in edgelist to a random permumtation of edgelist. Specifically, graph_morph.cpp permutes by relabeling every node in the edge list file to another node randomly, generating a 1 to 1 random mapping.
