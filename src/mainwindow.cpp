
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2023 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// md-editor include.
#include "mainwindow.hpp"
#include "editor.hpp"
#include "webview.hpp"
#include "previewpage.hpp"
#include "htmldocument.hpp"

// Qt include.
#include <QSplitter>
#include <QWidget>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QWebChannel>

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/parser.hpp>
#include <md4qt/html.hpp>


namespace MdEditor {

//
// MainWindowPrivate
//

struct MainWindowPrivate {
	MainWindowPrivate( MainWindow * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		auto w = new QWidget( q );
		auto l = new QHBoxLayout( w );
		splitter = new QSplitter( Qt::Horizontal, w );
		editor = new Editor( w );
		preview = new WebView( w );

		splitter->addWidget( editor );
		splitter->addWidget( preview );

		l->addWidget( splitter );

		q->setCentralWidget( w );
		q->setWindowTitle( MainWindow::tr( "Markdown Editor" ) );

		page = new PreviewPage( q );
		preview->setPage( page );

		html = new HtmlDocument( q );

		auto channel = new QWebChannel( q );
		channel->registerObject( QStringLiteral( "content" ), html );
		page->setWebChannel( channel );

		preview->setUrl( QUrl( "qrc:/res/index.html" ) );

		QObject::connect( editor, &QPlainTextEdit::textChanged,
			[this] ()
			{
				auto md = editor->toPlainText();
				QTextStream stream( &md );

				MD::Parser< MD::QStringTrait > parser;
				const auto doc = parser.parse( stream, editor->docName() );

				html->setText( MD::toHtml( doc ) );
			} );

		editor->setDocName( QStringLiteral( "default.md" ) );
	}

	MainWindow * q = nullptr;
	Editor * editor = nullptr;
	WebView * preview = nullptr;
	PreviewPage * page = nullptr;
	QSplitter * splitter = nullptr;
	HtmlDocument * html = nullptr;
	bool init = false;
}; // struct MainWindowPrivate


//
// MainWindow
//

MainWindow::MainWindow()
	:	d( new MainWindowPrivate( this ) )
{
	d->initUi();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::resizeEvent( QResizeEvent * e )
{
	if( !d->init )
	{
		d->init = true;

		auto w = centralWidget()->width() / 2;

		d->splitter->setSizes( { w, w } );
	}

	e->accept();
}

} /* namespace MdEditor */
