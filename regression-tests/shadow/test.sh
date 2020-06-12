#!/bin/sh
TMPDIR=/tmp/regression-shadow.$$
trap "/bin/rm -rf $TMPDIR" 0 1 2 3 15
mkdir $TMPDIR

echo "WARNING: TODO: ADD CHECKS ACCORDING TO THE COMMENTS IN THIS FILE ($0)" >&1

#creates a shadow from random alignments for 4 versions of the yeast network
./$SANA_EXE.multi -mode shadow -fshadow $REG_DIR/yeastStartShadow.txt > $TMPDIR/start.stdout 2>&1
#most edges should have weight 1 (because it's unlikely that 2 edges will be aligned by chance in sparse graphs)
#but some edges with weight 2 or even higher are possible
#the total edge weight is 39116

#creates a shadow from the same alignment for 4 versions of the yeast network
./$SANA_EXE.multi -mode shadow -fshadow $REG_DIR/yeastNextShadow.txt > $TMPDIR/next.stdout 2>&1
#most edges should have weight 4 (because most edges are repeated across all 4 graphs)
#but some edges with weight 3, 2, and 1 are expected because some of the versions are bigger than others
#it should have around 1/4th of the edges from the previos case
#but total edge weight does not change: it is still 39116
#the graph should consist of exactly 111 CCs: 1 for the shadow nodes mapped to by the yeast nodes,
#and 100 for the extra nodes which are not aligned by anyone because all the alignments are the same


./$SANA_EXE.multi -mode shadow -fshadow $REG_DIR/yeastStartShadowWithCols.txt > $TMPDIR/start.cols.stdout 2>&1
#what some statistics should show:
#adjLists size: 1114
#color group sizes: 1090 23 1
#totalEdgeWeight: 39116

./$SANA_EXE.multi -mode shadow -fshadow $REG_DIR/yeastNextShadowWithCols.txt > $TMPDIR/next.cols.stdout 2>&1
#the generated color file for the shadow graph should be exactly the same as in the previous case
#these don't change:
#adjLists size: 1114
#color group sizes: 1090 23 1
#totalEdgeWeight: 39116
