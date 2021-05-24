#-----------------------------------------------------------------------------
# VMTK
#-----------------------------------------------------------------------------

if(MITK_USE_VMTK)
  set(proj VMTK)
  set(proj_DEPENDENCIES ITK VTK)
  set(${proj}_DEPENDS ${proj})

  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()
  
  if(NOT DEFINED ${proj}_DIR)
    set(additional_cmake_args)

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_VTK_VMTK_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()
	
	if(MITK_USE_Python)
    if(NOT MITK_USE_SYSTEM_PYTHON)
     list(APPEND proj_DEPENDENCIES Python)
     set(_vmtk_install_python_dir -DVMTK_INSTALL_PYTHON_MODULE_DIR:FILEPATH=${MITK_PYTHON_SITE_DIR})
    else()
      set(_vmtk_install_python_dir -DVMTK_INSTALL_PYTHON_MODULE_DIR:PATH=${ep_prefix}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
    endif()

    list(APPEND additional_cmake_args
         -DVMTK_WRAP_PYTHON:BOOL=ON
         -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
         -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
         -DPYTHON_INCLUDE_DIR2:PATH=${PYTHON_INCLUDE_DIR2}
         -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
         ${_vmtk_install_python_dir}
        )
  else()
    list(APPEND additional_cmake_args
         -DVMTK_WRAP_PYTHON:BOOL=OFF
		 )
  endif()
  
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL "https://github.com/Crimson-MITK-ThirdParty/vtkVmtk/archive/refs/tags/vtk6-CRIMSON.tar.gz"
	  PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/VMTK.patch
	  INSTALL_COMMAND ""

      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        -DVTK_VMTK_BUILD_STREAMTRACER:BOOL=OFF
        -DVTK_VMTK_BUILD_TETGEN:BOOL=ON
        -DVTK_VMTK_WRAP_PYTHON:BOOL=ON
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR ${ep_prefix})
    #mitkFunctionInstallExternalCMakeProject(${proj})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
