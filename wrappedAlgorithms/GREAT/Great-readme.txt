Place all networks into the networks folder. The scripts are programmed to get the networks from this folder.
The networks must be in *.gw format.
The GW graphs must have the following format:

LEDA.GRAPH
string
short
3
|{1}|
|{2}|
|{3}|
3
1 2 0 |{}|
2 3 0 |{}|
3 1 0 |{}|

You must compute edge-GDVs and place the *.ecounts file into the orbits folder. This can be done by using the count.sh file as follows: ./count <gw file> [note: do not include the .gw extension]
Also note that any instance of "-" in the node names is replaced with "=", as the "-" character is used to identify edges. If you do not wish to use the "=", you can change it in the src/rename.py file.
There are 6 variations of GREAT, and two of these variations use MI-GRAAL.

The scripts for the remaining 4 variations are included:
run_G_G.sh aligns edges greedily, and nodes greedily
run_G_H.sh aligns edges greedily, and nodes optimally
run_H_H.sh aligns edges optimally, and nodes optimally
run_H_G.sh aligns edges optimally, and nodes greedily

run_G_G.sh and run_H_H.sh produces a file (located in the Migraal_Scores folder in the greated G-G and H-H directory) that can be used to produce MI-GRAAL results.

After running a script, you input the name of the smaller network, then the larger network, then the alpha parameter. [Note, put each variable on its own line, that is, press [enter] after entering each variable]
The alpha parameter should be between 0 and 1, where 0 is just edge-GDC, and 1 is just edge-GDV.

Alpha parameters should be entered as follows:
alpha = 0 should be inputted as 00
alpha = 1 should be inputted as 10

