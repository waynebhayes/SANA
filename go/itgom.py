#made by Vikram Saraph

import requests
from bs4 import BeautifulSoup
bs = BeautifulSoup
import re
from sys import argv
from time import sleep
from copy import copy

url = 'http://web.cbio.uct.ac.za/ITGOM/tools/itgomEngine.php'
url_get = 'http://web.cbio.uct.ac.za/ITGOM/tools/upload/'
delay = 2
gotermsfile = open(argv[1])
blocksize = 2999
timeouttime = 15
extn = ".trm"
outfile = open(argv[1].replace(extn, '.sim'), 'w')

b = 0
first_pass = True
unprocessed = set()
while 1:
	if first_pass:
		gotermslist = [gotermsfile.readline().strip() for i in xrange(blocksize)]
		goterms = '\n'.join(gotermslist) + '\n'
		if not gotermslist[0]:
			first_pass = False
			continue
	elif unprocessed:
		b, goterms = unprocessed.pop()
	else:
		break
	payload = {'SMeasure': '2', 'SOntology': '1', 'SFamily': '1', 'IncludeIEA': '1', 'SApproach': '9', 'Submit': 'Submit', 'UserData': goterms}
	try:
		r = requests.post(url, data=payload, timeout=timeouttime)
	except requests.exceptions.Timeout:
		print "Timeout %d, will retry later" % b
		unprocessed.add((b, copy(goterms)))
		if first_pass:
			b += 1
		continue
	if r.status_code != 200:
		print "Problem with server"
		break
	soup = bs(r.content)
	res = soup.findAll('a', text=re.compile('here'))
	try:
		suffix = str(res[0]).split("\"")[1]
	except IndexError:
		print "bad block"
		continue
	s = requests.get(url_get + suffix)
	print url_get + suffix
	if s.status_code != 200:
		print "Problem with server"
		break
	outfile.write(s.content)
	print "Block %d done." % b
	if first_pass:
		b += 1
	sleep(delay)
outfile.close()
