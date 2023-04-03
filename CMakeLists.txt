
cmake_minimum_required( VERSION 3.19 )

project( md-editor )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin )

find_package( Qt6Core REQUIRED )

if( UNIX AND NOT APPLE )
	list( PREPEND CMAKE_INSTALL_RPATH "${QT6_INSTALL_PREFIX}/lib:\$ORIGIN/../lib" )
else()
	list( PREPEND CMAKE_INSTALL_RPATH "${QT6_INSTALL_PREFIX}/lib" )
	list( PREPEND CMAKE_INSTALL_RPATH "@executable_path/../lib" )
	list( PREPEND CMAKE_INSTALL_RPATH "@loader_path/../lib" )
endif()

set( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
set( CMAKE_MACOSX_RPATH TRUE )

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
