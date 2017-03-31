SET(PACKAGE_NAME ${PROJECT_NAME})
SET(PACKAGE_VERSION ${PROJECT_VERSION})
SET(abs_top_srcdir ${PROJECT_SOURCE_DIR})
SET(abs_top_builddir ${PROJECT_BINARY_DIR})

SET(LEMON_DOC_SOURCE_BROWSER "NO" CACHE STRING "Include source into the doc (YES/NO).")

CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
  ${PROJECT_BINARY_DIR}/doc/Doxyfile
  @ONLY
)

CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/doc/mainpage.dox.in
  ${PROJECT_BINARY_DIR}/doc/mainpage.dox
  @ONLY
)

IF(DOXYGEN_EXECUTABLE AND PYTHONINTERP_FOUND AND GHOSTSCRIPT_EXECUTABLE)
  FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/)
  SET(GHOSTSCRIPT_OPTIONS -dNOPAUSE -dBATCH -q -dEPSCrop -dTextAlphaBits=4 -dGraphicsAlphaBits=4 -sDEVICE=pngalpha)
  ADD_CUSTOM_TARGET(html
    COMMAND ${CMAKE_COMMAND} -E remove_directory gen-images
    COMMAND ${CMAKE_COMMAND} -E make_directory gen-images
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/bipartite_matching.png ${CMAKE_CURRENT_SOURCE_DIR}/images/bipartite_matching.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/bipartite_partitions.png ${CMAKE_CURRENT_SOURCE_DIR}/images/bipartite_partitions.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/connected_components.png ${CMAKE_CURRENT_SOURCE_DIR}/images/connected_components.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/edge_biconnected_components.png ${CMAKE_CURRENT_SOURCE_DIR}/images/edge_biconnected_components.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/grid_graph.png ${CMAKE_CURRENT_SOURCE_DIR}/images/grid_graph.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/matching.png ${CMAKE_CURRENT_SOURCE_DIR}/images/matching.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/node_biconnected_components.png ${CMAKE_CURRENT_SOURCE_DIR}/images/node_biconnected_components.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/nodeshape_0.png ${CMAKE_CURRENT_SOURCE_DIR}/images/nodeshape_0.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/nodeshape_1.png ${CMAKE_CURRENT_SOURCE_DIR}/images/nodeshape_1.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/nodeshape_2.png ${CMAKE_CURRENT_SOURCE_DIR}/images/nodeshape_2.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/nodeshape_3.png ${CMAKE_CURRENT_SOURCE_DIR}/images/nodeshape_3.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/nodeshape_4.png ${CMAKE_CURRENT_SOURCE_DIR}/images/nodeshape_4.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/planar.png ${CMAKE_CURRENT_SOURCE_DIR}/images/planar.eps
    COMMAND ${GHOSTSCRIPT_EXECUTABLE} ${GHOSTSCRIPT_OPTIONS} -r18 -sOutputFile=gen-images/strongly_connected_components.png ${CMAKE_CURRENT_SOURCE_DIR}/images/strongly_connected_components.eps
    COMMAND ${CMAKE_COMMAND} -E remove_directory html
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/bib2dox.py ${CMAKE_CURRENT_SOURCE_DIR}/references.bib >references.dox
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  SET_TARGET_PROPERTIES(html PROPERTIES PROJECT_LABEL BUILD_DOC)

  IF(UNIX)
    INSTALL(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
      DESTINATION share/doc/lemon/html
      COMPONENT html_documentation
    )
  ELSEIF(WIN32)
    INSTALL(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
      DESTINATION doc
      COMPONENT html_documentation
    )
  ENDIF()

ENDIF()

IF(WGET_FOUND)
ADD_CUSTOM_TARGET(update-external-tags
  COMMAND ${CMAKE_COMMAND} -E make_directory dl
  # COMMAND ${CMAKE_COMMAND} -E copy libstdc++.tag dl
  COMMAND ${WGET_EXECUTABLE} wget -P dl -N libstdc++.tag.tmp http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag
  COMMAND ${CMAKE_COMMAND} -E rename dl/libstdc++.tag libstdc++.tag
  COMMAND ${CMAKE_COMMAND} -E remove dl/libstdc++.tag
  COMMAND ${CMAKE_COMMAND} -E remove_directory dl
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
ENDIF()
