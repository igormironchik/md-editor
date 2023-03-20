
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
	gotoline.hpp
	gotoline.cpp
	gotoline.ui
	fontdlg.hpp
	fontdlg.cpp
	fontdlg.ui
	${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp )

qt6_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC md-editor.rc )
endif()

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	PRE_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../bin/cfgfile.generator${CMAKE_EXECUTABLE_SUFFIX} -i cfg.cfgconf -o ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp PROPERTY SKIP_AUTOGEN ON )

add_definitions( -DCFGFILE_QT_SUPPORT )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
	${md4qt_INCLUDE_DIRECTORIES}
	${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cfgfile
	${CMAKE_CURRENT_BINARY_DIR} )

add_executable( md-editor WIN32 ${SRC} )

add_dependencies( md-editor cfgfile.generator )

target_link_libraries( md-editor Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core )
