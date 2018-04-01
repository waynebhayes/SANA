Placeholder README for visualization_graph_morph

currently the expected input is:

For transformation: python visualization_graph_morph -t transition_number(integer) edgelist1(file path) edgelist2_file(file path) output_file(file path)
For combination: python visualization_graph_morph -c weight1(float). . . weightn(float) edgelist1(file path) . . .  edgelist2(file path)


Libraries used are networkx, matplotlib, numpy, scipy

ImageMagick is needed since the script generates .gif files for transformation