optimizedNSGAII:
	icc nsgaiimain.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o nsgaii -g -prof-use -ipo -prof-dir. -use-intel-optimized-headers -tbb -par-num-threads=16 -opt-mem-layout-trans=3 -opt-subscript-in-range -ansi-alias -xHost -unroll-aggressive -opt-calloc -no-prec-div -stdlib=libc++ -O3 -Wall -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_thread-mt -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -pthread

NSGAII:
	icc nsgaiimain.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o nsgaii -use-intel-optimized-headers -tbb -par-num-threads=16 -opt-mem-layout-trans=3 -opt-subscript-in-range -ansi-alias -xHost -unroll-aggressive -opt-calloc -no-prec-div -stdlib=libc++ -O3 -Wall -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_thread-mt -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -pthread

profileNSGAII:
	icpc nsgaiimain.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o nsgaii -O2 -prof-gen -prof-dir. -tbb -ldl -Wall -std=c++11 -g -stdlib=libc++ -I /usr/include/ -L /usr/lib/x86_64-linux-gnu/ -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -lboost_thread-mt	
ubuntuNSGAII:
	icc nsgaiimain.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o nsgaii -prof-use -ipo -prof-dir. -use-intel-optimized-headers -tbb -par-num-threads=8 -opt-mem-layout-trans=3 -opt-subscript-in-range -ansi-alias -fno-alias -O3 -xHost -unroll-aggressive -opt-calloc -no-prec-div -Wall -std=c++11 -I /usr/include/ -L /usr/lib/x86_64-linux-gnu/ -lboost_filesystem -lboost_system -lboost_program_options -lboost_thread	

test:
	clang++ tests.cpp blastinfo.cpp Alignment.cpp Network.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o test -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_unit_test_framework-mt

testubuntu:
	clang++ tests.cpp blastinfo.cpp Alignment.cpp Network.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o test -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_unit_test_framework


hillclimber:
	icc hillclimber.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o hillclimber -stdlib=libc++ -O3 -Wall -tbb -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_thread-mt -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -pthread

hillclimberUbuntu:
	icc hillclimber.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp goc.cpp -o hillclimber -stdlib=libc++ -unroll-aggressive -xHost -use-intel-optimized-headers -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS -Wall -tbb -g -std=c++11 -I /usr/include/ -L /usr/lib/x86_64-linux-gnu/ -lboost_filesystem -lboost_system -lboost_program_options -lboost_thread


optnetalign:
	icc steadystate.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp Archive.cpp goc.cpp -o optnetalign  -use-intel-optimized-headers -opt-mem-layout-trans=3 -opt-subscript-in-range -xHost -unroll-aggressive -O3 -DNDEBUG -tbb -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_program_options-mt

optnetalignOptimized:
	icc steadystate.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp Archive.cpp goc.cpp -o optnetalign -prof-use -ipo -prof-dir. -use-intel-optimized-headers -opt-mem-layout-trans=3 -opt-subscript-in-range -xHost -unroll-aggressive -O3 -DNDEBUG -tbb -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_program_options-mt

optnetalignprof:
	icc steadystate.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp Archive.cpp goc.cpp -o optnetalign -O2 -prof-gen -prof-dir. -tbb -std=c++11 -I /usr/local/Cellar/boost/1.53.0/include/ -L /usr/local/Cellar/boost/1.53.0/lib -lboost_program_options-mt


optnetalignubuntu:
	g++ steadystate.cpp Network.cpp blastinfo.cpp Alignment.cpp nsga-ii.cpp localSearch.cpp Archive.cpp goc.cpp -o optnetalign -g -std=c++11 -I /usr/include/ -L /usr/lib/x86_64-linux-gnu/ -lboost_program_options -lpthread
