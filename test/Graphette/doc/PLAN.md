Goal: Search for a graphette in a graph
=========================================
```C++
vector<vector<bool>> orbitSig(vector<bool>(totalOrbitNum,0), nodeNum)
for i < NUM_SAMPLE
	Graphette sample = G.getSampleGraphette(k);
	vector<uint> orbitId = sample.getOrbitIds();
	for j < g.getNumNodes()
		orbitSig[g.getLabel(j)][orbitId[j]]=true;
```
Properties of Graphette Class
-----------------------------