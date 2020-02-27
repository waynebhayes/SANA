import pstats, sys
pfile = sys.argv[1]
mode = sys.argv[2]
numstats = sys.argv[3]

if numstats is None:
    numstats = 20
else:
    numstats = int(numstats)

p = pstats.Stats(pfile)
p.sort_stats(mode).print_stats(numstats)


