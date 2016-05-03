import random

for i in xrange(500):
	mutswappb = random.uniform(0.0001,0.009)
	cxswappb = random.uniform(0.1,1.0)
	popsize = random.randint(100,500)
	tournsel = random.choice([True,False])
	total = random.choice([True,False])
	smallstart = False if total else random.choice([True,False])
	uniformsize = False if total or smallstart else random.choice([True,False])
	print "./optnetalign --net1 ~/Dropbox/optnetalign/optnetalign/tests/dmela.net --net2 ~/Dropbox/optnetalign/optnetalign/tests/hsapi.net --bitscores ~/Dropbox/optnetalign/optnetalign/tests/dm-hs.sim --ics --mutswappb " + str(mutswappb) + " --cxswappb " + str(cxswappb) + " --popsize " + str(popsize) + " " + ("--tournsel " if tournsel else " ") + ("--smallstart " if smallstart else " ") + (" --uniformsize " if uniformsize else " ") + "--nthreads 16 --outprefix foo --generations 1000 --finalstats >> experimentalData.csv"
	print "rm *.aln"
	print "rm *.info"