#!/bin/bash
echo "Testing SANA's ability to create many pairwise alignments and output only the best one"
./sana "Leo: put the appropriate arguments here" -o regression-tests/PairwiseParallel/test-best
echo "Now compare correct outputs to the test outputs"

cmp regression-tests/PairwiseParallel/test-best.out regression-tests/PairwiseParallel/test-best-correct.out
cmp regression-tests/PairwiseParallel/test-best.align regression-tests/PairwiseParallel/test-best-correct.align

