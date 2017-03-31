#! /usr/bin/env python
#
# This file is a part of LEMON, a generic C++ optimization library.
#
# Copyright (C) 2003-2009
# Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
# (Egervary Research Group on Combinatorial Optimization, EGRES).
#
# Permission to use, modify and distribute this software is granted
# provided that this copyright notice appears in all copies. For
# precise terms see the accompanying LICENSE file.
#
# This software is provided "AS IS" with no warranty of any kind,
# express or implied, and with no claim as to its suitability for any
# purpose.

import sys

from mercurial import ui, hg
from mercurial import util

util.rcpath = lambda : []

if len(sys.argv)>1 and sys.argv[1] in ["-h","--help"]:
    print """
This utility just prints the length of the longest path
in the revision graph from revison 0 to the current one.
"""
    exit(0)

u = ui.ui()
r = hg.repository(u, ".")
N = r.changectx(".").rev()
lengths=[0]*(N+1)
for i in range(N+1):
    p=r.changectx(i).parents()
    if p[0]:
        p0=lengths[p[0].rev()]
    else:
        p0=-1
    if len(p)>1 and p[1]:
        p1=lengths[p[1].rev()]
    else:
        p1=-1
    lengths[i]=max(p0,p1)+1
print lengths[N]
