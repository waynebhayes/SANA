cmake_minimum_required( VERSION 2.8 )

project( natalie )

set( Analyze_Hdr
  src/analysis/analyze.h
  src/analyzealignment.h
)

set( Analyze_Src
  src/analyzealignment.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( AnalyzeGO_Hdr
  src/analysis/analyze.h
  src/analysis/gocollection.h
  src/analyzego.h
)

set( AnalyzeGO_Src
  src/analyzego.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( Tinyxml_Hdr
  src/externlib/tinyxml/tinystr.h
  src/externlib/tinyxml/tinyxml.h
)

set( Tinyxml_Src
  src/externlib/tinyxml/tinystr.cpp
  src/externlib/tinyxml/tinyxml.cpp
  src/externlib/tinyxml/tinyxmlerror.cpp
  src/externlib/tinyxml/tinyxmlparser.cpp
)

set( Gml-parser_Hdr
  src/externlib/gml-parser/gml_parser.h
  src/externlib/gml-parser/gml_scanner.h
)

set( Gml-parser_Src
  src/externlib/gml-parser/gml_parser.c
  src/externlib/gml-parser/gml_scanner.c
)

set( Demo_Src
  src/demo.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( Demo_Hdr
  src/gna.h
  src/natalie.h
  src/input/matchinggraph.h
  src/verbose.h
  src/webserver.h
)

set( Makegraphs_Hdr
  src/gna.h
  src/natalie.h
  src/input/matchinggraph.h
  src/verbose.h
  src/webserver.h
)

set( Makegraphs_Src
  src/makegraphs.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( Natalie-cpx_Hdr
  src/gna.h
  src/cplex/cplexgna.h
  src/cplex/cplexgnald.h
  src/cplex/cplexgnaqp.h
  src/cplex/cplexgnasi.h
)

set( Natalie-cpx_Src
  src/nataliecplex.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( Natalie_Hdr
  src/gna.h
  src/natalie.h
  src/verbose.h
  src/webserver.h
  src/score/scoremodel.h
  src/score/scoremodelcoexpressionfunction.h
  src/score/scoremodelcoexpressiondiscrete.h
  src/score/scoremodelcoexpressioncontinuous2.h
  src/score/scoremodelcoexpressioncontinuous.h
  src/score/scoremodeledgecorrectness.h
  src/score/scoremodelbitscore.h
  src/score/scoremodelcoexpressiondiscreteinverse.h
  src/output/output.h
  src/output/outputcsv.h
  src/output/outputdot.h
  src/output/outputgml.h
  src/output/outputjson.h
  src/output/outputlgf.h
  src/output/outputneato.h
  src/output/outputsif.h
  src/output/outputeda.h
  src/output/outputnoa.h
  src/output/outputparameters.h
  src/output/outputstat.h
  src/output/outputanalyse.h
  src/output/outputparametersamc.h
  src/output/outputstatamc.h
  src/output/outputcompstatamc.h
  src/output/outputcsvamc.h
  src/output/outputjsonamc.h
  src/lagrange/combinedsolver.h
  src/lagrange/dualdescentsolver.h
  src/lagrange/lagrangeitfc.h
  src/lagrange/lagrangesolver.h
  src/lagrange/subgradientsolver.h
  src/lagrange/lagrangegnasicached.h
  src/lagrange/lagrangegnasi.h
  src/lagrange/lagrangegnaldcached.h
  src/lagrange/lagrangegnald.h
  src/lagrange/lagrangegnacached.h
  src/lagrange/lagrangegna.h
  src/lagrange/globalproblem.h
  src/lagrange/globalproblembase.h
  src/lagrange/globalproblemconstrained.h
  src/input/stringparser.h
  src/input/parser.h
  src/input/matchinggraph.h
  src/input/lgfparser.h
  src/input/ledaparser.h
  src/input/graphmlparser.h
  src/input/gmlparser.h
  src/input/csvparser.h
  src/input/bpparser.h
  src/input/bpcandlistparser.h
  src/input/bpblastparser.h
  src/input/bplgfparser.h
  src/input/identityparser.h
  src/input/bpidentityparser.h
  src/input/edgelistparser.h
  src/input/matchinggraphshuffle.h
)

set( Natalie_Src
  src/natalie.cpp
  src/verbose.cpp
  src/webserver.cpp
)

set( Common_Hdr
  src/input/matchinggraph.h
  src/score/scoremodel.h
)

set( Common_Src
  src/verbose.cpp
)

set( GitCommand1 "symbolic-ref HEAD 2> /dev/null | cut -b 12-" )
execute_process( COMMAND git symbolic-ref HEAD  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 COMMAND tail -c -7 COMMAND tr -d "\\n" OUTPUT_VARIABLE GitBranch )
set( GitCommand2 "log --pretty=format:\\\"%h\\\" -1" )
execute_process( COMMAND git log --pretty=format:%h WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 COMMAND head -n 1  COMMAND tr -d "\\n" OUTPUT_VARIABLE GitHashtag )
set( NATALIE_VERSION \"${GitBranch}-${GitHashtag}\" )

set( LIBLEMON_ROOT "$ENV{HOME}/lemon" CACHE PATH "Additional search directory for lemon library" )

if( NOT CPLEX_INC_DIR OR NOT CPLEX_LIB_DIR OR NOT CONCERT_INC_DIR OR NOT CONCERT_LIB_DIR )
	set( CPLEX_AUTODETECTION_PREFIXES "$ENV{HOME}/ILOG;/opt/ibm/ILOG;$ENV{HOME}/Applications/IBM/ILOG" CACHE STRING "CPLEX automatic lookup directories" )
	foreach( CplexInstallPrefix ${CPLEX_AUTODETECTION_PREFIXES} )
		file( GLOB_RECURSE CplexLibraryPaths ${CplexInstallPrefix}/*/cplex/lib/*/libcplex.a )
		if( CplexLibraryPaths )
			list( LENGTH CplexLibraryPaths NumPath )
			if( NumPath GREATER 1 )
				message( WARNING "Found multiple CPLEX installations in ${CplexInstallPrefix}, automatically selecting the first one, please set {CPLEX|CONCERT}_{INC|LIB}_DIR manually if this is not the right selection" )
			else()
				message( "Found CPLEX's installation in ${CPlexInstallPrefix}" )
			endif()
			list( GET CplexLibraryPaths 0 CplexLibraryPath )

			set( MyRegex "${CplexInstallPrefix}/(.*)/cplex/?/lib/(.*)/libcplex.a" )
			string( REGEX MATCH "${MyRegex}" ValidatedCplexLibPath ${CplexLibraryPath} )
			if( NOT ValidatedCplexLibPath )
				message( WARNING "Unable to recognize ${CplexLibraryPath} as a valid CPLEX path" )
			else()
				string( REGEX REPLACE "${MyRegex}" "\\1" CplexHome ${ValidatedCplexLibPath} )
				string( REGEX REPLACE "${MyRegex}" "\\2" CplexArch ${ValidatedCplexLibPath} )

				set( CPLEX_INC_DIR   "${CplexInstallPrefix}/${CplexHome}/cplex/include"            CACHE PATH "CPLEX's include dir" )
				set( CPLEX_LIB_DIR   "${CplexInstallPrefix}/${CplexHome}/cplex/lib/${CplexArch}"   CACHE PATH "CPLEX's library dir" )
				set( CONCERT_INC_DIR "${CplexInstallPrefix}/${CplexHome}/concert/include"          CACHE PATH "Concert's include dir" )
        set( CONCERT_LIB_DIR "${CplexInstallPrefix}/${CplexHome}/concert/lib/${CplexArch}" CACHE PATH "Concert's library dir" )

				break()
			endif()
		else()
			message( "Unable to find CPLEX and Concert installation in ${CplexInstallPrefix}" )
		endif()
	endforeach()

	if( NOT CPLEX_INC_DIR OR NOT CPLEX_LIB_DIR OR NOT CONCERT_INC_DIR OR NOT CONCERT_LIB_DIR )
		message( "Unable to autodetect CPLEX's and Concert's installation directories, please set {CPLEX|CONCERT}_{INC|LIB}_DIR manually" )
	else()
		message( "Autodetected CPLEX's include dir   : ${CPLEX_INC_DIR}" )
		message( "Autodetected CPLEX's library dir   : ${CPLEX_LIB_DIR}" )
		message( "Autodetected Concert's include dir : ${CONCERT_INC_DIR}" )
		message( "Autodetected Concert's library dir : ${CONCERT_LIB_DIR}" )
	endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release Profile" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wno-long-long -Wno-unused-parameter -Wno-unknown-pragmas -g -ggdb -O0 -DDEBUG -DVERBOSE")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wno-long-long -Wno-unused-parameter -Wno-unknown-pragmas -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_PROFILE "-Wall -Wextra -Wno-long-long -Wno-unused-parameter -Wno-unknown-pragmas -O3 -g -ggdb -DNDEBUG")

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings " )
add_definitions( -DIL_STD )
configure_file( src/config.h.in ${PROJECT_SOURCE_DIR}/src/config.h )

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  #set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ " )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ " )
endif()

include_directories( ${CPLEX_INC_DIR} ${CONCERT_INC_DIR} "${LIBLEMON_ROOT}/include" "src" "src/externlib" )
link_directories( ${CPLEX_LIB_DIR} ${CONCERT_LIB_DIR} "${LIBLEMON_ROOT}/lib" )

set( CommonLibs
	emon
)

set( CplexLibs
  m
  ilocplex
  cplex
  concert
  pthread
)

add_executable( analyze ${Analyze_Hdr} ${Analyze_Src} ${Common_Hdr} ${Common_Src} ${CommonObjs} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} )
target_link_libraries( analyze ${CommonLibs} )

#add_executable( analyzeGO ${AnalyzeGO_Hdr} ${AnalyzeGO_Src} ${Common_Hdr} ${Common_Src} ${CommonObjs} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} )
#target_link_libraries( analyzeGO ${CommonLibs} )

add_executable( natalie ${Natalie_Hdr} ${Natalie_Src} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} ${CommonObjs} )
target_link_libraries( natalie ${CommonLibs} )

add_executable( make-graphs ${Makegraphs_Hdr} ${Makegraphs_Src} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} ${CommonObjs} )
target_link_libraries( make-graphs ${CommonLibs} )

add_executable( demo ${Demo_Hdr} ${Demo_Src} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} ${CommonObjs} )
target_link_libraries( demo ${CommonLibs} )

if( CPLEX_INC_DIR AND CPLEX_LIB_DIR AND CONCERT_INC_DIR AND CONCERT_LIB_DIR )
  add_executable( natalie-cpx ${Natalie-cpx_Hdr} ${Natalie-cpx_Src} ${Tinyxml_Hdr} ${Tinyxml_Src} ${Gml-parser_Hdr} ${Gml-parser_Src} ${CommonObjs} )
  target_link_libraries( natalie-cpx ${CommonLibs} ${CplexLibs} )
endif()

