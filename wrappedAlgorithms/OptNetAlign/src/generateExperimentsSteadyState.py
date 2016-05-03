import random

for i in xrange(500):
	mutswappb = random.uniform(0.0001,0.001)
	[mutrate] = random.sample([0.0, 0.5],1)
	cxswappb = random.uniform(0.1,1.0)
	[cxrate] = random.sample([0.0,0.5],1)
	hillclimbiters = int(random.uniform(5000, 50000))
	dynparams = True
	popsize = int(random.uniform(5,1000))
	print "./steadystateMOGA --net1 ~/Dropbox/optnetalign/optnetalign/tests/dmela.net --net2 ~/Dropbox/optnetalign/optnetalign/tests/hsapi.net --annotations1 ~/Dropbox/optnetalign/optnetalign/tests/dmela.annos --annotations2 ~/Dropbox/optnetalign/optnetalign/tests/hsapi.annos --goc --s3 " + " --hillclimbiters " + str(hillclimbiters) + " --mutswappb " + str(mutswappb) + " --mutrate " + str(mutrate) + " --cxswappb " + str(cxswappb) + " --cxrate " + str(cxrate) + " --popsize " + str(popsize) + (" --dynparams " if dynparams else " ")  + "--total --nthreads 16 --nooutput --generations 10000000 --timelimit 120 --finalstats >> experimentalDataSteadyState.csv"