#-----------------------------------------------------------------------------
# CGAL
#-----------------------------------------------------------------------------

if(MITK_USE_CGAL)

  # Sanity checks
  if(DEFINED CGAL_ROOT AND NOT EXISTS ${CGAL_ROOT})
    message(FATAL_ERROR "CGAL_ROOT variable is defined but corresponds to non-existing directory")
  endif()

  string(REPLACE "^^" ";" MITK_USE_CGAL_LIBRARIES "${MITK_USE_CGAL_LIBRARIES}")

  set(proj CGAL)
  set(proj_DEPENDENCIES )
  set(CGAL_DEPENDS ${proj})

  if(NOT DEFINED CGAL_ROOT AND NOT MITK_USE_SYSTEM_CGAL)

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL https://gforge.inria.fr/frs/download.php/file/34898/CGAL-4.6.1.tar.gz
      #URL_MD5 a744cf167b05d72335f27c88115f211d
      # We use in-source builds for CGAL
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      -DCGAL_DISABLE_GMP:BOOL=ON
      -DWITH_CGAL_ImageIO:BOOL=OFF
      -DWITH_CGAL_Qt3:BOOL=OFF
      -DWITH_CGAL_Qt4:BOOL=OFF
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} install_dir)

    if(WIN32)
      set(CGAL_LIBRARYDIR "${install_dir}/lib")
    endif()

    ExternalProject_Get_Property(${proj} binary_dir)
    set(CGAL_DIR ${binary_dir})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
