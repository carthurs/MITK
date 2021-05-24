
set(_module_deps MitkDiffusionCore MitkGraphAlgorithms)

mitk_check_module_dependencies(
  MODULES ${_module_deps}
  MISSING_DEPENDENCIES_VAR _missing_deps
)

if(NOT _missing_deps)
  set(lut_url https://github.com/Crimson-MITK-ThirdParty/FiberTrackingLUT/archive/refs/tags/1.0.tar.gz)
  set(lut_tarball ${CMAKE_CURRENT_BINARY_DIR}/FibertrackingLUT.tar.gz)
  message("Downloading FiberTracking LUT ${lut_url}...")
  file(DOWNLOAD ${lut_url} ${lut_tarball}
       TIMEOUT 600 #600 seconds, or 10 minutes
       STATUS status
       SHOW_PROGRESS
      )

  list(GET status 0 status_code)
  list(GET status 1 status_msg)

  if(NOT status_code EQUAL 0)
    message(SEND_ERROR "${status_msg} (error code ${status_code})")
  else()
    message("done.")
  endif()

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)
  message("Unpacking FiberTracking LUT tarball...")
  execute_process(COMMAND ${CMAKE_COMMAND} -E  tar xzf ../FibertrackingLUT.tar.gz
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources
                  RESULT_VARIABLE result
                  ERROR_VARIABLE err_msg)
  if(result)
    message(SEND_ERROR "Unpacking FibertrackingLUT.tar.gz failed: ${err_msg}")
  else()
    message("done.")
  endif()

endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Fiberfox Fiberfox/SignalModels Fiberfox/Sequences Algorithms Algorithms/MLTracking Algorithms/GibbsTracking Algorithms/StochasticTracking IODataStructures IODataStructures/FiberBundle IODataStructures/PlanarFigureComposite Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS PUBLIC ITK|ITKFFT ITK|ITKDiffusionTensorImage Vigra HDF5
  #WARNINGS_AS_ERRORS
)

if(MODULE_IS_ENABLED)
  add_subdirectory(Testing)
endif()
