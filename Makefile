CC = g++

#CXXFLAGS = -I "src/utils" -Wall -fno-inline -O2 -std=c++11 -g3
#CXXFLAGS = -I "src/utils" -U__STRICT_ANSI__ -Wall -std=c++11 -O3 -DMULTI_PAIRWISE #-pg -ggdb -Bstatic #-static
 CXXFLAGS = -I "src/utils" -U__STRICT_ANSI__ -Wall -std=c++11 -O3 -pthread #-ggdb -DMULTI_PAIRWISE #-DCORES #-DUSE_CACHED_FILES #-DSPARSE -ggdb #-pg

ifeq ($(SPARSE), 1)
CXXFLAGS := $(CXXFLAGS) -DSPARSE
endif

ifeq ($(MULTI), 1)
CXXFLAGS := $(CXXFLAGS) -DMULTI_PAIRWISE
endif


INCLUDES =
LFLAGS =
LIBS =

METHODS_SRC =   							\
	src/methods/SANA.cpp    					\
	src/methods/GreedyLCCS.cpp 					\
	src/methods/HillClimbing.cpp    				\
	src/methods/Method.cpp  					\
	src/methods/NoneMethod.cpp 					\
	src/methods/RandomAligner.cpp   				\
	src/methods/TabuSearch.cpp 					\
	src/methods/WeightedAlignmentVoter.cpp  			\
	src/methods/Dijkstra.cpp

UTILS_SRC = 								\
	src/utils/NormalDistribution.cpp				\
	src/utils/templateUtils.cpp					\
	src/utils/Timer.cpp						\
	src/utils/utils.cpp						\
	src/utils/randomSeed.cpp					\
	src/utils/SkipList.cpp						\
	src/utils/SeedMatrix.cpp					\
	src/utils/LinearRegression.cpp					\
	src/utils/ParetoFront.cpp                                       \
	src/utils/Matrix.cpp

ARGUMENTS_SRC = 							\
	src/arguments/ArgumentParser.cpp				\
	src/arguments/modeSelector.cpp					\
	src/arguments/measureSelector.cpp				\
	src/arguments/methodSelector.cpp				\
	src/arguments/graphLoader.cpp					\
	src/arguments/supportedArguments.cpp				\
	src/arguments/defaultArguments.cpp

MEASURES_SRCS = 							\
	src/measures/EdgeCorrectness.cpp 				\
	src/measures/EdgeDifference.cpp         			\
	src/measures/EdgeRatio.cpp         				\
	src/measures/TriangleCorrectness.cpp				\
	src/measures/MultiEdgeCorrectness.cpp 				\
	src/measures/ExternalWeightedEdgeConservation.cpp               \
	src/measures/GoAverage.cpp      				\
	src/measures/NetGO.cpp      					\
	src/measures/InducedConservedStructure.cpp			\
	src/measures/InvalidMeasure.cpp             			\
	src/measures/LargestCommonConnectedSubgraph.cpp			\
	src/measures/Measure.cpp					\
	src/measures/MeasureCombination.cpp				\
	src/measures/NodeCorrectness.cpp 				\
	src/measures/ShortestPathConservation.cpp 			\
	src/measures/SymmetricSubstructureScore.cpp 			\
	src/measures/SymmetricEdgeCoverage.cpp 				\
	src/measures/SquaredEdgeScore.cpp 				\
	src/measures/EdgeExposure.cpp 				\
	src/measures/WeightedEdgeConservation.cpp 			\
	src/measures/JaccardSimilarityScore.cpp         			\
	src/measures/MultiS3.cpp							\
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
	src/methods/wrappers/CytoGEDEVOWrapper.cpp			\

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
	src/modes/DebugMode.cpp						\
	src/modes/ClusterMode.cpp					\
	src/modes/AnalysisMode.cpp					\
	src/modes/SimilarityMode.cpp					\
	src/modes/ParetoMode.cpp

OTHER_SRC = 							\
	src/Graph.cpp                                       	\
	src/computeGraphlets.cpp                            	\
	src/Alignment.cpp                                   	\
	src/complementaryProteins.cpp                       	\
	src/main.cpp                                        	\
	src/report.cpp


SRCS = $(METHODS_SRC) $(OTHER_SRC) $(UTILS_SRC) $(MEASURES_SRCS) $(METHOD_WRAPPERS_SRC) $(SCHEDULEMETHODS_SRC) $(ARGUMENTS_SRC) $(MODES_SRC)
OBJDIR = _objs
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

#MAIN = sana_dbg
MAIN = sana

.PHONY: depend clean test test_all regression_test

all:    $(MAIN) argumentCSV createShadow NetGO parallel

$(MAIN): $(OBJS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

$(MAIN).static: $(OBJS)
	$(CC) $(CXXFLAGS) -static $(INCLUDES) -o $(MAIN).static $(OBJS) $(LFLAGS) $(LIBS)

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
	$(RM) -rf $(OBJDIR)/src
	$(RM) -f $(MAIN) createShadow parallel
	$(RM) -f src/arguments/argumentTable.csv

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

NetGO:
	(cd NetGO && git pull)

multi:
	$(MAKE) 'MULTI=1'
	mv sana sana.multi

sparse:
	$(MAKE) 'SPARSE=1'
	mv sana sana.sparse

createShadow: scripts/createShadowCpp/createShadow
	mv scripts/createShadowCpp/createShadow .

parallel: src/parallel.c
	gcc -o parallel src/parallel.c
