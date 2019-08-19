add_library(sana_lib 
    # Graph Types
    Graph/BinaryGraph.cpp 
    Graph/WeightedGraph.cpp
    Graph/Graph.cpp

    # SANA Measures
    Measures/EdgeCorrectness.cpp
    Measures/ExternalWeightedEdgeConservation.cpp
    Measures/GoAverage.cpp
    Measures/GoSimilarity.cpp
    Measures/Measure.cpp
    Measures/SymmetricSubstructureScore.cpp

    # SANA Configuration
    Configuration/SANAConfiguration.cpp

    # Alignments
    Alignment/PairwiseAlignment.cpp

    # Result
    Result/SANAResult.cpp

    # TemperatureSchedules
    TemperatureSchedule/LinearRegression.cpp
    TemperatureSchedule/TemperatureSchedule.cpp
    # Core
    Core/SANA.cpp

    # Utilities
    Utils/Timer.cpp
    Utils/Random.cpp 
    Utils/Utils.cpp
)

target_include_directories(sana_lib
    PUBLIC
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_options(sana_lib PRIVATE -Werror)
target_compile_features(sana_lib PRIVATE cxx_std_17)

if (ENABLE_CPPCHECK)
    add_cppcheck(sana_lib)
endif()

