
cmake_minimum_required( VERSION 3.19 )

project( md-editor )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

link_directories( ${CMAKE_BINARY_DIR}/lib )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build."
		FORCE)
endif( NOT CMAKE_BUILD_TYPE )

set( CMAKE_CXX_STANDARD 17 )

set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( BUILD_MD4QT_QT_TESTS OFF CACHE INTERNAL "" FORCE )
add_subdirectory( 3rdparty/md4qt )

set( BUILD_WIDGETS_EXAMPLES OFF CACHE INTERNAL "" FORCE )
add_subdirectory( 3rdparty/widgets )

add_subdirectory( 3rdparty/cfgfile/generator )
add_subdirectory( src )
