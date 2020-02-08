import pstats, sys
pfile = sys.argv[1]
mode = sys.argv[2]

p = pstats.Stats(pfile)
p.sort_stats(mode).print_stats(20)


