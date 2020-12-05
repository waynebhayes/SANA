CC = g++
CXXFLAGS = -I "src/utils" -U__STRICT_ANSI__ -Wall -std=c++11 -pthread #-pg -fno-inline

MAIN = sana

#you can give these on Make's command line, eg "SPARSE=1" or "FLOAT=1" or "MULTI=1"
#all can be mixed and matched except FLOAT and MULTI

ifeq ($(SPARSE), 1)
    CXXFLAGS := $(CXXFLAGS) -DSPARSE
    MAIN := $(MAIN).sparse
endif

ifeq ($(CORES), 1)
    CXXFLAGS := $(CXXFLAGS) -DCORES
    MAIN := $(MAIN).cores
endif

ifeq ($(MULTI), 1)
    CXXFLAGS := $(CXXFLAGS) -DMULTI_PAIRWISE
    MAIN := $(MAIN).multi
endif

ifeq ($(FLOAT), 1)
    CXXFLAGS := $(CXXFLAGS) -DFLOAT_WEIGHTS
    MAIN := $(MAIN).float
endif

ifeq ($(STATIC), 1)
    CXXFLAGS := $(CXXFLAGS) -Bstatic #-static for some versions of gcc
    MAIN := $(MAIN).static
endif

ifeq ($(GDB), 1)
    CXXFLAGS := $(CXXFLAGS) -ggdb
    MAIN := $(MAIN).gdb
else
    CXXFLAGS := $(CXXFLAGS) -O3
endif

######## THIS ONE MUST BE LAST to ensure "MAIN=error" when an incompatible combination occurs ##################
ifeq ($(MULTI), 1)
    ifeq ($(FLOAT), 1)
	ERROR="SANA cannot currently use FLOAT in MULTI-alignments"
	MAIN=error
    endif
    ifeq ($(CORES), 1)
	ERROR="SANA cannot currently compute CORES in MULTI-alignments"
	MAIN=error
    endif
endif

INCLUDES =
LFLAGS =
LIBS =

METHODS_SRC =   							\
	src/methods/SANA.cpp    					\
	src/methods/HillClimbing.cpp    				\
	src/methods/Method.cpp  					\
	src/methods/NoneMethod.cpp 					\
	src/methods/RandomAligner.cpp   				

UTILS_SRC = 								\
	src/utils/NormalDistribution.cpp				\
	src/utils/templateUtils.cpp					\
	src/utils/Timer.cpp						\
	src/utils/utils.cpp						\
	src/utils/FileIO.cpp						\
	src/utils/randomSeed.cpp					\
	src/utils/LinearRegression.cpp					\
	src/utils/computeGraphlets.cpp                            	\
	src/utils/ComputeGraphletsWrapper.cpp				\
	src/utils/Matrix.cpp						\
	src/utils/SANAversion.cpp

ARGUMENTS_SRC = 							\
	src/arguments/ArgumentParser.cpp				\
	src/arguments/modeSelector.cpp					\
	src/arguments/measureSelector.cpp				\
	src/arguments/MethodSelector.cpp				\
	src/arguments/GraphLoader.cpp					\
	src/arguments/SupportedArguments.cpp				

MEASURES_SRCS = 							\
	src/measures/EdgeCorrectness.cpp 				\
	src/measures/EdgeDifference.cpp         			\
	src/measures/EdgeRatio.cpp         				\
	src/measures/MultiEdgeCorrectness.cpp 				\
	src/measures/ExternalWeightedEdgeConservation.cpp               \
	src/measures/NetGO.cpp      					\
	src/measures/InducedConservedStructure.cpp			\
	src/measures/InvalidMeasure.cpp             			\
	src/measures/LargestCommonConnectedSubgraph.cpp			\
	src/measures/Measure.cpp					\
	src/measures/MeasureCombination.cpp				\
	src/measures/NodeCorrectness.cpp 				\
	src/measures/SymmetricSubstructureScore.cpp 			\
	src/measures/SymmetricEdgeCoverage.cpp 				\
	src/measures/SquaredEdgeScore.cpp 				\
	src/measures/EdgeExposure.cpp 				\
	src/measures/WeightedEdgeConservation.cpp 			\
	src/measures/JaccardSimilarityScore.cpp         			\
	src/measures/MultiS3.cpp							\
	src/measures/CoreScore.cpp						\
	src/measures/localMeasures/EdgeCount.cpp 			\
	src/measures/localMeasures/EdgeDensity.cpp 			\
	src/measures/localMeasures/ExternalSimMatrix.cpp 		\
	src/measures/localMeasures/GenericLocalMeasure.cpp 		\
	src/measures/localMeasures/GoSimilarity.cpp 			\
	src/measures/localMeasures/Graphlet.cpp 			\
	src/measures/localMeasures/GraphletLGraal.cpp 			\
	src/measures/localMeasures/Importance.cpp 			\
	src/measures/localMeasures/LocalMeasure.cpp 			\
	src/measures/localMeasures/NodeCount.cpp 			\
	src/measures/localMeasures/NodeDensity.cpp 			\
	src/measures/localMeasures/Sequence.cpp 			\
	src/measures/localMeasures/GraphletCosine.cpp 			\
	src/measures/localMeasures/GraphletNorm.cpp

METHOD_WRAPPERS_SRC =    						\
	src/methods/wrappers/WrappedMethod.cpp				\
	src/methods/wrappers/HubAlignWrapper.cpp 			\
	src/methods/wrappers/LGraalWrapper.cpp  			\
	src/methods/wrappers/NETALWrapper.cpp				\
	src/methods/wrappers/MIGRAALWrapper.cpp				\
	src/methods/wrappers/GHOSTWrapper.cpp				\
	src/methods/wrappers/PISwapWrapper.cpp				\
	src/methods/wrappers/OptNetAlignWrapper.cpp			\
	src/methods/wrappers/SPINALWrapper.cpp				\
	src/methods/wrappers/GREATWrapper.cpp				\
	src/methods/wrappers/NATALIEWrapper.cpp				\
	src/methods/wrappers/GEDEVOWrapper.cpp				\
	src/methods/wrappers/MagnaWrapper.cpp				\
	src/methods/wrappers/WAVEWrapper.cpp				\
	src/methods/wrappers/PINALOGWrapper.cpp				\
	src/methods/wrappers/SANAPISWAPWrapper.cpp			\
	src/methods/wrappers/CytoGEDEVOWrapper.cpp			

SCHEDULEMETHODS_SRC =   							\
	src/schedulemethods/Ameur.cpp 					\
	src/schedulemethods/IteratedAmeur.cpp    				\
	src/schedulemethods/LinearRegressionModern.cpp  					\
	src/schedulemethods/LinearRegressionVintage.cpp 					\
	src/schedulemethods/PBadBinarySearch.cpp   				\
	src/schedulemethods/ScheduleMethod.cpp    					\
	src/schedulemethods/scheduleUtils.cpp 					\
	src/schedulemethods/StatisticalTest.cpp  	\
	src/schedulemethods/IteratedLinearRegression.cpp		

MODES_SRC = 								\
	src/modes/AlphaEstimation.cpp 					\
	src/modes/Experiment.cpp 					\
	src/modes/ParameterEstimation.cpp 				\
	src/modes/NormalMode.cpp					\
	src/modes/CreateShadow.cpp					\
	src/modes/DebugMode.cpp						\
	src/modes/ClusterMode.cpp					\
	src/modes/AnalysisMode.cpp					\
	src/modes/SimilarityMode.cpp					

OTHER_SRC = 							\
	src/Graph.cpp                                       	\
	src/Alignment.cpp                                   	\
	src/complementaryProteins.cpp                       	\
	src/main.cpp                                        	\
	src/Report.cpp

SRCS = $(METHODS_SRC) $(OTHER_SRC) $(UTILS_SRC) $(MEASURES_SRCS) $(METHOD_WRAPPERS_SRC) $(SCHEDULEMETHODS_SRC) $(ARGUMENTS_SRC) $(MODES_SRC)
OBJDIR = _objs
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

.PHONY: depend clean test test_all regression_test

all:   parallel NetGO argumentCSV $(MAIN)

ifeq ($(MAIN), error)
error:
	echo "ERROR: $(ERROR)">&2; exit 1
else
$(MAIN): $(OBJS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
endif

#.c.o:
#	$(CC) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

#$(OBJDIR)/%.o: %.c
#    $(CC) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c $(INCLUDES) -o $@ $< $(CXXFLAGS)

# TEST_SRC is anything that ends in _UT (unit test)
TEST_SRC = $(wildcard test/*_UT.cpp)
# TEST_DEPENDS is all the OBJS without main and Gtest
_MAIN_OBJ = $(OBJDIR)/src/main.o
TEST_DEPENDS = $(filter-out $(_MAIN_OBJ), $(OBJS))
GTEST_OBJS = test/gtest/gtest-main.o test/gtest/gtest-all.o
TEST_OBJS = $(addprefix $(OBJDIR)/, $(TEST_SRC:.cpp=.o))
TEST_MAIN = unit_test
TEST_CXXFLAGS = $(CXXFLAGS) -lpthread # add pthread for Gtest

regression_test:
	./regressionTest.sh

argumentCSV:
	(cd src/arguments; ./create-csv.sh)

test_all: $(TEST_OBJS) $(GTEST_OBJS) $(TEST_DEPENDS)
	$(CC) $(TEST_CXXFLAGS) $(INCLUDES) -o $(OBJDIR)/$(TEST_MAIN) $(TEST_OBJS) $(GTEST_OBJS) $(TEST_DEPENDS) $(LFLAGS) $(LIBS)
	./$(OBJDIR)/$(TEST_MAIN)

test: $(OBJDIR)/test/$(tg).o $(GTEST_OBJS) $(TEST_DEPENDS)
	$(CC) $(TEST_CXXFLAGS) $(INCLUDES) -o $(OBJDIR)/$(tg) $(OBJDIR)/test/$(tg).o $(GTEST_OBJS) $(TEST_DEPENDS) $(LFLAGS) $(LIBS)
	./$(OBJDIR)/$(tg)

$(GTEST_OBJS):
	cd test/gtest && make

clean: #clear_cache
	$(RM) -rf cache*  # mostly for pBad
	$(RM) -rf $(OBJDIR)/src $(MAIN).exe $(MAIN)
	$(RM) -f src/arguments/argumentTable.csv src/utils/SANAversion.cpp
	$(RM) -f *.exe.stackdump core.[0-9]* # core dumps on CYGWIN + Unix, respectively

clear_cache:
	rm -rf autogenerated
	rm -rf matrices/autogenerated
	rm -rf tmp
	rm -rf networks/*/autogenerated
	rm -rf cache_pbad

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# Here is the targets for making the wrappedAlgorithms
optnetalign:
	cd wrappedAlgorithms/OptNetAlign/src; $(MAKE) optnetalignubuntu; chmod +x optnetalign; cp optnetalign ../

NetGO: /dev/null # always make
	git submodule init
	git submodule update
	(cd NetGO && git checkout master && git pull)

parallel: src/parallel.c
	gcc -o parallel src/parallel.c

src/utils/SANAversion.cpp: .git/HEAD .git/index Makefile
	echo "const char *SANAversion = \"Version$(shell git show -s '--format=%d %H %ci') Compiled with gcc/g++ \" __VERSION__;" > $@
