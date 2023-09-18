
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package( Qt6Core REQUIRED )
find_package( Qt6Widgets REQUIRED )
find_package( Qt6WebEngineWidgets REQUIRED )

set( SRC main.cpp
	editor.hpp
	editor.cpp
	mainwindow.hpp
	mainwindow.cpp
	webview.hpp
	webview.cpp
	previewpage.hpp
	previewpage.cpp
	htmldocument.hpp
	htmldocument.cpp
	find.hpp
	find.cpp
	find.ui
	findweb.hpp
	findweb.cpp
	findweb.ui
	gotoline.hpp
	gotoline.cpp
	gotoline.ui
	fontdlg.hpp
	fontdlg.cpp
	fontdlg.ui
	${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	closebutton.hpp
	closebutton.cpp )

qt6_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC md-editor.rc )
endif()

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	PRE_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../bin/cfgfile.generator${CMAKE_EXECUTABLE_SUFFIX} -i cfg.cfgconf -o ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process( COMMAND git rev-parse --short HEAD
	OUTPUT_VARIABLE MDEDITOR_VERSION_MINOR
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_STRIP_TRAILING_WHITESPACE )

set( MDEDITOR_VERSION "0.1.18 build ${MDEDITOR_VERSION_MINOR}" )

configure_file( version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.hpp )

set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp PROPERTY SKIP_AUTOGEN ON )

add_definitions( -DCFGFILE_QT_SUPPORT )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
	${md4qt_INCLUDE_DIRECTORIES}
	${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cfgfile
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/widgets/include )

add_executable( md-editor WIN32 ${SRC} )

add_dependencies( md-editor cfgfile.generator )

target_link_libraries( md-editor widgets Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core )
