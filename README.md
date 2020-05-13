# SANA
This is the Simulating Annealing Network Aligner, designed originally to align protein-protein interaction (PPI) networks, but now is able to align arbitrary undirected graphs, weighted on unweighted.
It can produce near-optimal alignments of PPI-sized networks in about an hour, and it has been used to successfully align networks with up to 100,000 nodes and 100 million edges (though those alignments take about a week of CPU to complete).

For details, consult our original paper: https://doi.org/10.1093/bioinformatics/btx090
A tutorial introduction is available at https://arxiv.org/abs/1911.10133


## Remaining TODO LIST for SANA 2.0 refactor
1- Move the incremental evaluation logic to each Measure, and use runtime polymorphism to run the appropriate logic for the "active" measures included in MeasureCombination. There is a very nice way to do this incrementally and safely. Start with one measure, say, EC, and add the new logic without removing the existing one. Then, assert at the end of each iteration that the old and new logics coincide. If they do (e.g, across the test suite), the old logic can be safely removed. This can be done measure by measure until SANA is cleaned up. Our other email chain on the subject can serve as basis for the design philosophy to follow to do this.  

2- Insert virtual dummy nodes to handle yin-yang problem in pairwise alignment. Of course, it is always possible to add dummy nodes manually or use multi pairwise with just 2 graphs. The advantage of doing it natively would be efficiency, as it is possibly to not waste time swapping between dummy nodes and still sample neighbor solutions uniformly. However, I don't like that this feature introduces a new concept to the SANA class (virtual dummy nodes) and makes it more complex. For this reason, I wanted to implement (1) before doing this.
