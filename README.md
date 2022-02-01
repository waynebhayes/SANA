# SANA
Note: when cloning, you should get the submodules too, using:

    git clone --recurse-submodules https://github.com/waynebhayes/SANA

This is the Simulating Annealing Network Aligner, designed originally to align protein-protein interaction (PPI) networks, but now is able to align arbitrary undirected graphs, weighted on unweighted.
It can produce near-optimal alignments of PPI-sized networks in about an hour, and it has been used to successfully align networks with up to 100,000 nodes and 100 million edges (though those alignments take about a week of CPU to complete).

For slightly out-of-date details, consult our original paper: https://doi.org/10.1093/bioinformatics/btx090
A tutorial introduction is available at https://arxiv.org/abs/1911.10133

If you want to do development on SANA, you should read the the following documents:
    src/STYLE_GUIDE.txt - how to integrate your code into SANA seamlessly
    doc/addingMeasures/README.md - how to add a new measure to SANA
    
## Specifying the measure to optimize, and the runtime to choose.
You will note that the above tutorial is out-of-date. In particular, SANA now *demands* that you specify both the objective(s) to optimize (eg., EC, S^3, etc.), as well as the amount of time to spend optimizing it (-t XXX argument, where XXX is in minutes). 

This is because choosing which measure you want to optimize, and how much time to spend on it, turn out to be so *incredibly* important, that we now **force** users to think about the issue.  There are literally dozens of topological measures you could optimize, and it's crucial to understand what you want to get out of the alignment before choosing one.

For example: Do you expect there to be significant global similarity between the two networks?  In the biological case, this would be when, for example, both PPI networks come from the same species but measured by different labs or at different times. If this is the case then there are really only two options.

a) If you expect one network is significantly less dense (in edges) than the other, then use EC.

b) If both networks have roughly similar edge density (ie., both are missing about the same amount of edges), then use s3.

If you do *not* expect the networks to be similar, it gets quite a bit more complicated, and to offer advice I (Wayne B. Hayes) would need to know quite a bit more about what you're attempting to make a recommendation on which objective you should optimize.

In terms of run-time.... there's no a-priori way to determine, in advance, how much run time may be necessary to get a good answer. The solution we've come up with so far is to perform self alignments. That means, give the same network as both G1 and G2. Then run SANA optimizing S3 for longer-and-longer times until the s3 score reaches exactly 1.0, or very very close to it (above 0.99, for example). For example start at 1 minute and keep doubling. For example, PPI networks with 6,000 nodes and 100,000 edges can be self-aligned in under an hour, eg at 16 minutes you'll get an S3 score of exactly 1.) But other networks with the same number of nodes may require more, or less, time.

# If you plan to change the code....
... then read the file src/README.md for the style guide.
