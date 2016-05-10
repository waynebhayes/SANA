natalie
=======

Global pairwise network alignment

Dependencies
------------

* LEMON 1.3
* Optional: ILOG CPLEX (>= 12.0)

Compiling
---------

Get natalie from github:

    git clone <HTTPS clone URL (see on the right side of this page)>


First, LEMON 1.3 needs to be installed:

    wget http://lemon.cs.elte.hu/pub/sources/lemon-1.3.tar.gz
    tar xvzf lemon-1.3.tar.gz
    cd lemon-1.3
    cmake -DCMAKE_INSTALL_PREFIX=~/lemon
    make install
    
Note: On Mac OS 10.9, comment out the following two lines and add the code below at line 162 in `CMakeLists.txt` before `make install`

    #ADD_SUBDIRECTORY(demo) 
    #ADD_SUBDIRECTORY(tools)
    
    if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ " )
    endif()

You can remove the LEMON sources now, i.e., `rm -rf lemon-1.3`. Next, natalie can be compiled:

    mkdir build
    cd build
    cmake ..
    make

In case auto-detection of LEMON or CPLEX fails, do

    cmake \
    -DLIBLEMON_ROOT=~/lemon \
    -DCPLEX_INC_DIR=~/ILOG/cplex/include/ \
    -DCPLEX_LIB_DIR=~/ILOG/cplex/lib/x86-64_osx/static_pic \
    -DCONCERT_LIB_DIR=~/ILOG/concert/lib/x86-64_osx/static_pic \
    -DCONCERT_INC_DIR=~/ILOG/concert/include/ ..

Running natalie
---------------

To run natalie:

    ./natalie -if1 0 -if2 0 -ifm 0 -g1 ../data/rno.gml -g2 ../data/hsa.gml -gm ../data/rno_hsa.seqSim
    ./natalie-cpx -if1 0 -if2 0 -ifm 0 -g1 ../data/rno.gml -g2 ../data/hsa.gml -gm ../data/rno_hsa.seqSim

For usage instructions specify `-h`.
