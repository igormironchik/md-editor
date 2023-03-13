
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
	find.cpp )

qt6_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND GUI_SRC md-editor.rc )
endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
	${md4qt_INCLUDE_DIRECTORIES} )

add_executable( md-editor WIN32 ${SRC} )

target_link_libraries( md-editor Qt6::WebEngineWidgets Qt6::Widgets Qt6::Core )
