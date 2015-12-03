#------------------------------------------------------------------
# LIBPNG
#------------------------------------------------------------------
if(MITK_USE_matplotlib_LIBPNG)
  if(NOT DEFINED matplotlib_LIBPNG_DIR)
    set(proj matplotlib_LIBPNG)
    set(${proj}_DEPENDENCIES matplotlib_ZLIB)
    set(matplotlib_LIBPNG_DEPENDS ${proj})

    set(additional_cmake_args )
    
    if (MSVC)
        set(matplotlib_LIBPNG_build_command ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release) # always build release
        set(matplotlib_LIBPNG_install_command ${CMAKE_COMMAND} -E copy <BINARY_DIR>/Release/libpng16_static${CMAKE_IMPORT_LIBRARY_SUFFIX} <BINARY_DIR>/png${CMAKE_IMPORT_LIBRARY_SUFFIX}) 
    else()
        set(matplotlib_LIBPNG_build_command ${CMAKE_COMMAND} --build <BINARY_DIR>)
        set(matplotlib_LIBPNG_install_command ${CMAKE_COMMAND} -E copy <BINARY_DIR>/libpng16.a <BINARY_DIR>/png.a) 
    endif()

    # Using the LIBPNG from CTK:
    # https://github.com/commontk/LIBPNG
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      BUILD_COMMAND ${matplotlib_LIBPNG_build_command}
      INSTALL_COMMAND ${matplotlib_LIBPNG_install_command} 
      URL https://www.dropbox.com/s/osup9o963i87y8t/libpng-1.6.19.tar.gz 
      #URL_MD5 "685ce2ebbf4ad9a03992fb5f10c38df1"
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        -DZLIB_LIBRARY:FILEPATH=${matplotlib_ZLIB_LIBRARY}
        -DZLIB_INCLUDE_DIR:FILEPATH=${matplotlib_ZLIB_INCLUDE_DIR}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DPNG_SHARED=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${matplotlib_LIBPNG_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} source_dir)
    ExternalProject_Get_Property(${proj} binary_dir)  
    set(matplotlib_LIBPNG_INCLUDE_DIR "${source_dir}${sep}${binary_dir}")    
    set(matplotlib_LIBPNG_LIBRARY_DIR ${binary_dir})    
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

