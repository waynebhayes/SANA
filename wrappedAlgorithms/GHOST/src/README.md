Introduction
============

GHOST is a program that aligns two networks using an algorithm detailed in "Global Network Alignment Using Multiscale Spectral Signatures" by Rob Patro and Carl Kingsford. The paper can be found at (http://bioinformatics.oxfordjournals.org/content/early/2012/10/08/bioinformatics.bts592.full.pdf+html). This version of the code is written in C++ and is faster than the previous version (see https://github.com/rob-p/GHOST). Currently, it has only been built on Windows (using Cygwin) and Unix machines.

Building
========

To use this code, first download it from the [git
repository](https://github.com/Kingsford-Group/ghost2). You will need to
download and install the C++ [Boost library](www.boost.org/users/download) if
you do not currently have it installed from. If Boost is not installed, please
read the Installing Boost section before proceeding. In addition, make sure
that your g++ is recent enough that is supports c++11. The version GHOST was
built with was 4.8.2, but you can check by running g++ -dumpversion in a
terminal and googling.

Now, using a terminal navigate to the directory that you unzipped from and
execute the following commands:

```
  $ cd GHOST-CPP
  $ export BOOSTLIB=<PATH_TO_BOOSTLIB>
  $ export BOOSTDIR=<PATH_TO_BOOSTDIR>
  $ make
```

where `<PATH_TO_BOOSTLIB>` and `<PATH_TO_BOOSTDIR>` are the absolute paths to
the lib folder containing the boost libaries (ex. libboost_thread.a) and the
include folder containing the boost folder with all of the .hpp files. If it
errors, it is possible that your ld library path is not set to the right place.
Trying something like:

```
  $ export LD_LIBRARY_PATH=/opt/local/lib:$LD_LIBRARY_PATH
```

may help. If all has went well, you should see an executable file named GHOST
in the directory now.

Installing Boost
================

The following instructions have been tested on a fresh installation of Ubuntu
14.04 LTS.

First download Boost from
[www.boost.org/user/download](www.boost.org/user/download) and zlib from
[www.zlob.net](www.zlib.net). zlib is necessary to build one of the parts of
boost properly. Unzip both into your home directory. Now, open a terminal and
execute the following commands:

```
  $ sudo apt-get install vim g++ bzip2 python-dev libxml2-dev libbz2-dev
  $ cd zlib-1.2.8
  $ ./configure; make test
  $ sudo make install
  $ cd ../boost_1_55_0
  $ ./boostrap.sh
  $ sudo ./b2 install
```

If all went well, Boost will be installed. It is likely that you will be able
run make now.

File Formats
============

Configuration Format
--------------------

The configuration file tells the program what you want it to do. It will look
something like:

```
  [main]
  network1: Data/CJejuni/cjejuni.gexf
  network2: Data/EColi/ecoli.gexf
  sequencescores: Data/CJejuni_vs_EColi.evalues
  hops: 4
```

The configuration file must have [main] as its first line and must include
values for network1 and network2. In addition, there should be no spaces
preceding or following the text in each line. You can set each option by
putting that option on a line followed by a ':' character, a space, and the
appropriate value. The options are as follows:

```
  NAME - DESCRIPTION

  STANDARD OPTIONS
  ****************

  network1 - The .gexf or .net file describing the first network that is to 
             be aligned.
  network2 - The .gexf or .net file describing the second network that is to 
             be aligned.
  sequencescores - The .evalues file containing the precomputed BLAST evalues.
  hops - The positive integer value for the radius of the subgraphs. The 
         default value is 4.
  nneighbors - The positive integer value for the number of nearest neighbors
               in H that each node in G will have a distance computed to.
               The default value is "all" which is coded as -1.
  searchiter - The positive integer value for the number of local search
               iterations that should be performed after the initial alignment
               is complete. The process will terminate early if the last loop
               yielded no changes. The default value is 10.
  alpha - The positive decimal value for the weight of the BLAST evalues in 
          the distance function. The value should be between 0 and 1 and be 
          written in decimal format (ex. 0.5). If no user-suggested alpha is 
          provided, GHOST will generate one as defined in the paper.
  beta - The positive decimal value for the hard constraint on sequence 
         similarity of aligned pairs. The default value is 1.0.
  ratio - The positive decimal value for the ratio of "bad-moves" allowed
          during the local-searh phase of the alignment algorithm. This is
          equivalent to the "budget" parameter described in the GHOST paper.
          The default value is 8.0.
  processors - The positive integer value for the number of processors to use. 
               If this is not set, GHOST will automatically generate what 
               should be the optimal value.

  PRECOMPUTATION OPTIONS
  **********************

  sigs1 - The .sig.gz file containing the precomputed signatures for network1.
  sigs2 - The .sig.gz file containing the precomputed signatures for network2
  distFile - The .sdf file containing the precomputed distances.
  alignFile - The .af file containing the precomputed alignment.
  dumpSignatures - The boolean value (true or false) for whether you want the 
                   computation to stop after computing the signatures. Note 
                   that the boolean values are lower case. The default value 
                   is false.
  dumpDistances - The boolean value (true or false) for whether you want the 
                  computation to stop after computing the distances. Note that 
                  the boolean values are lower case. The default value is false.

  EXTRA FUNCTIONALITY OPTIONS
  ***************************
  
  directed - The boolean value (true or false) for whether the graph inputted
             is directed or not. *has not been tested yet* Default value is
             false.
  seedSkip - The positive decimal value for the percent chance GHOST has to
             temporarily skip a seed. Used to measure confidence of matchings.
             Default value is 0.
  sigApprox - The string for the name of the signature approximation method
              you want to use. The choices are "rayleigh" and "inverseIter".
              "rayleigh" should be about as twice as fast with some loss in 
              performance. "inverseIter" is slower and worse, but is included
              nonetheless.
```

The extension associated with this format is ".cfg".

Graph File Format (gexf)
------------------------

GHOST can use the GEXF format which can be found at
[http://gexf.net/format](http://gexf.net/format). The graph should have the
attribute title="gname" and the ids and values of each node should be unique.
The extension associated with this format is `.gexf`.

Graph File Format (net)
-----------------------

GHOST can also use the .net format which is composed of |1+V+E| where the first line has the number of verticies and edges:

```
  |V| |E|
```

The next ```|V|``` lines are of the form:

```
  a
```

where v is the name of the next vertex, and the last ```|E|``` lines are of the form:

```
  a b
```

where a and b are verticies in the graph. The extension associated with this format is ".net".

Alignment File Format
---------------------

For input graphs G and H, the alignment format that GHOST outputs has lines of
the form:

```
  a   b
```

where a is the name of a unique vertex from G, b is the name of a unique vertex
from H, and the number of lines is min(|G|, |H|). The extension associated with
this format is `.af`.

Spectral Distance Format
------------------------

For input graphs G and H, the spectral distance format that GHOST outputs has
lines of the form:

```
  a   b   d
```

where a is the name of a vertex from G, b is the name of a vertex from H, and d
is the spectral distance between them. There should be |G|*|H| lines. The
extension associated with this format is `.sdf`.

Signature Format
----------------

The signature format is not human readable. It is stored in a compressed format
and can be outputted for the sake of precomputation by setting dumpSignatures
to true in the configuration file. The extension associated with this format is
`.sig.gz`.

Usage
=====

First, make a config file in the folder (see file formats).

GHOST has the following options:

```
  -c    This option accepts the path to the configuration file as an argument. It should almost always be present when you run the program.
  -p    This option accepts the number of processors it should use as an argument. It is recommended that this is not set because the program automatically generates what should be the optimal number of processors. This can be set in the .cfg file.
  -k    This option accepts the radius from the each node the spectral signature should be computed. The default value is 4. This can be set in the .cfg file.
```

A example of an ordinary run is:

```
  $ ./GHOST -c cjejuni_vs_ecoli.cfg
```

Libraries Used
==============

The GHOST program uses the following free-to-use libraries and files:

  1. Boost [www.boost.org](www.boost.org)
  2. Eigen [eigen.tuxfamily.org](eigen.tuxfamily.org)
  3. threadpool [threadpool.sourceforge.net](threadpool.sourceforge.net)
  4. fastapprox [https://code.google.com/p/fastapprox](https://code.google.com/p/fastapprox)

