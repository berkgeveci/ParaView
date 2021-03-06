INCLUDE(ParaViewTestingMacros)

SET(_SUFFIX "")
IF(VISIT_BUILD_READER_GMV)
   SET(_SUFFIX "${_SUFFIX}_WithVisitBridge")
ENDIF(VISIT_BUILD_READER_GMV)

SET(GMV_READER_TESTS
  ${CMAKE_CURRENT_SOURCE_DIR}/GMVReaderBasicRead${_SUFFIX}.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/GMVReaderInvalidFiles${_SUFFIX}.xml
)
SET(GMVReaderBasicRead${_SUFFIX}_BREAK TRUE)
SET(GMVReaderInvalidFiles${_SUFFIX}_BREAK TRUE)

IF (PARAVIEW_DATA_ROOT AND PARAVIEW_BUILD_QT_GUI)
  add_client_tests("pv"
    BASELINE_DIR ${PARAVIEW_DATA_ROOT}/Baseline
    TEST_SCRIPTS ${GMV_READER_TESTS}
    LOAD_PLUGIN "GMVReader"
  )
  SET_PROPERTY(TEST "pv.GMVReaderInvalidFiles${_SUFFIX}" PROPERTY PASS_REGULAR_EXPRESSION "client0 process exited with code 0")

  add_client_server_tests("pvcs"
    BASELINE_DIR ${PARAVIEW_DATA_ROOT}/Baseline
    TEST_SCRIPTS ${GMV_READER_TESTS}
    LOAD_PLUGIN "GMVReader"
  )
  SET_PROPERTY(TEST "pvcs.GMVReaderInvalidFiles${_SUFFIX}" PROPERTY PASS_REGULAR_EXPRESSION "server process exited with code 0")
ENDIF (PARAVIEW_DATA_ROOT AND PARAVIEW_BUILD_QT_GUI)
