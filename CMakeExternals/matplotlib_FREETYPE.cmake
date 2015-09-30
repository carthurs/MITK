#------------------------------------------------------------------
# FREETYPE
#------------------------------------------------------------------
if(MITK_USE_matplotlib_FREETYPE)
  if(NOT DEFINED matplotlib_FREETYPE_DIR)
    set(proj matplotlib_FREETYPE)
    set(${proj}_DEPENDENCIES )
    set(matplotlib_FREETYPE_DEPENDS ${proj})

    set(additional_cmake_args )
    
    if (MSVC)
        set(matplotlib_FREETYPE_build_command ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release) # always build release
        set(matplotlib_FREETYPE_install_command ${CMAKE_COMMAND} -E copy <BINARY_DIR>/Release/freetype${CMAKE_IMPORT_LIBRARY_SUFFIX} <BINARY_DIR>/freetype${CMAKE_IMPORT_LIBRARY_SUFFIX}) 
    else()
        set(matplotlib_FREETYPE_build_command ${CMAKE_COMMAND} --build <BINARY_DIR>)
        set(matplotlib_FREETYPE_install_command "") 
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      BUILD_COMMAND ${matplotlib_FREETYPE_build_command}
      INSTALL_COMMAND ${matplotlib_FREETYPE_install_command} 
      URL http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.gz
      #URL_MD5 "4c3f572b487ae7947fd88ec363533bc5"
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DBUILD_SHARED_LIBS:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${matplotlib_FREETYPE_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} source_dir)
    set(matplotlib_FREETYPE_INCLUDE_DIR ${source_dir}/include)    
    ExternalProject_Get_Property(${proj} binary_dir)  
    set(matplotlib_FREETYPE_LIBRARY_DIR ${binary_dir})    
      
    #set(matplotlib_FREETYPE_DIR ${ep_prefix})
    #set(matplotlib_FREETYPE_INCLUDE_DIR ${matplotlib_FREETYPE_DIR})
    #
    #find_library(matplotlib_FREETYPE_LIBRARY NAMES freetype
    #             PATHS ${matplotlib_FREETYPE_DIR}
    #             PATH_SUFFIXES lib lib/Release
    #             NO_DEFAULT_PATH)
    #             
    #get_filename_component(matplotlib_FREETYPE_LIBRARY_DIR ${matplotlib_FREETYPE_LIBRARY} DIRECTORY)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

