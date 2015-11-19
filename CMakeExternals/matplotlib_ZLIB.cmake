#------------------------------------------------------------------
# ZLIB
#------------------------------------------------------------------
if(MITK_USE_matplotlib_ZLIB)
  if(NOT DEFINED matplotlib_ZLIB_DIR)
    include(${CMAKE_CURRENT_LIST_DIR}/ZLIB_URL.cmake)
    
    set(proj matplotlib_ZLIB)
    set(${proj}_DEPENDENCIES )
    set(matplotlib_ZLIB_DEPENDS ${proj})

    set(additional_cmake_args )
    
    if (MSVC)
        set(matplotlib_ZLIB_build_command ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release) # always build release
        set(matplotlib_ZLIB_install_command ${CMAKE_COMMAND} -E copy <BINARY_DIR>/Release/zlib${CMAKE_IMPORT_LIBRARY_SUFFIX} <BINARY_DIR>/z${CMAKE_IMPORT_LIBRARY_SUFFIX}) 
        set(matplotlib_ZLIB_library_name z${CMAKE_IMPORT_LIBRARY_SUFFIX})
    else()
        set(matplotlib_ZLIB_build_command ${CMAKE_COMMAND} --build <BINARY_DIR>)
        set(matplotlib_ZLIB_install_command ${CMAKE_COMMAND} -E copy <BINARY_DIR>/libzlib.a <BINARY_DIR>/z.a) 
        set(matplotlib_ZLIB_library_name z.a)
    endif()

    # Using the ZLIB from CTK:
    # https://github.com/commontk/ZLIB
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      BUILD_COMMAND ${matplotlib_ZLIB_build_command} 
      INSTALL_COMMAND ${matplotlib_ZLIB_install_command} 
      URL ${ZLIB_URL}
      URL_MD5 "4c3f572b487ae7947fd88ec363533bc5"
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DBUILD_SHARED_LIBS:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${matplotlib_ZLIB_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} source_dir)
    ExternalProject_Get_Property(${proj} binary_dir)  
    set(matplotlib_ZLIB_INCLUDE_DIR "${source_dir}${sep}${binary_dir}")    
    set(matplotlib_ZLIB_LIBRARY_DIR ${binary_dir})   
    set(matplotlib_ZLIB_LIBRARY ${binary_dir}/${matplotlib_ZLIB_library_name})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

