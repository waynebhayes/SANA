usage of WAVE:
WAVE arg1 arg2 arg3 arg4
Each argument indicates a file name:

arg1: 	graph G(V_G, E_G)
	The first line of the file contains two integers: the number of nodes (n) and the number of edges (m). Each node in this file is then denoted as an integer (the label starts from 0 to n-1).
	Each of the following m lines contains two integers u and v, representing that there is an edge in the graph that linking node u and and node v.

arg2: 	graph H(V_H, E_H)
	The same format as graph G. Notice that G is the (relatively) smaller graph, namely |V_G|<=|V_H|

arg3: 	similarity file
	Each line in this file contains 2 integers u and u', and a real value s, indicating that node u in V_G and node v in V_H has similarity of s. The similarity s could be pre-processed by any Node Cost Function (NCF). Note that s in this file indicates SIMILARITY, which means the larger the value, the higher probability to link corresponding nodes.

arg4:	output file
	Users can assign a name to output file. The alignment got by WAVE will be stored in this file.

