Goal: Search for a graphette in a graph
=========================================
```C++
vector<vector<bool>> orbitSig(nodeNum, vector<bool>(totalOrbitNum,0))
for i < NUM_SAMPLE
	Graphette sample = G.getSampleGraphette(k);
	vector<uint> orbitId = sample.getOrbitIds();
	for j < g.getNumNodes()
		orbitSig[g.getLabel(j)][orbitId[j]]=true;
```
Properties of Graphette Class
-----------------------------
###Note
I am tired of getters and setters -_-

This code bleed my eyes.

Number of possible nodes is 65535 since I have used uint.
the number of possible edges is (n*n-n)/2. this n*n exceeds the limit of
uint for n>=2^16