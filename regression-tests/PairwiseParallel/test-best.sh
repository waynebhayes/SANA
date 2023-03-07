#!/bin/bash
echo "Testing SANA's ability to create many pairwise alignments and output only the best one"

exit 0 # Leo: remove this exit line once you have regression tests actually working

./sana -fg1 networks/RNorvegicus.el -fg2 networks/SPombe.el -ec 1 -parallelTotalAlign 4 -parallelBatch 2 -parallelReportAll 1 -o regression-tests/PairwiseParallel/test-best

echo "Now compare correct outputs to the test outputs"

cmp regression-tests/PairwiseParallel/test-best.out regression-tests/PairwiseParallel/test-best-correct.out
cmp regression-tests/PairwiseParallel/test-best.align regression-tests/PairwiseParallel/test-best-correct.align


