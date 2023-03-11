
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
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QFileInfo>
#include <QDir>
#include <QToolTip>

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

		page = new PreviewPage( q );
		preview->setPage( page );

		html = new HtmlDocument( q );

		auto channel = new QWebChannel( q );
		channel->registerObject( QStringLiteral( "content" ), html );
		page->setWebChannel( channel );

		page->setHtml( q->htmlContent( "." ) );
		editor->setDocName( QStringLiteral( "default.md" ) );

		q->setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" ).arg( editor->docName() ) );

		auto fileMenu = q->menuBar()->addMenu( MainWindow::tr( "&File" ) );
		newAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-new.png" ) ),
				MainWindow::tr( "New" ), MainWindow::tr( "Ctrl+N" ), q, &MainWindow::onFileNew );
		openAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-open.png" ) ),
				MainWindow::tr( "Open" ), MainWindow::tr( "Ctrl+O" ), q, &MainWindow::onFileOpen );
		fileMenu->addSeparator();
		saveAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-save.png" ) ),
			MainWindow::tr( "Save" ), MainWindow::tr( "Ctrl+S" ), q, &MainWindow::onFileSave );
		saveAsAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-save-as.png" ) ),
			MainWindow::tr( "Save As" ), q, &MainWindow::onFileSaveAs );
		fileMenu->addSeparator();
		fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/application-exit.png" ) ),
			MainWindow::tr( "Quit" ), MainWindow::tr( "Ctrl+Q" ), q, &QWidget::close );

		auto helpMenu = q->menuBar()->addMenu( MainWindow::tr( "&Help" ) );
		helpMenu->addAction( QIcon( QStringLiteral( ":/res/img/icon_24x24.png" ) ),
			MainWindow::tr( "About" ), q, &MainWindow::onAbout );
		helpMenu->addAction( QIcon( QStringLiteral( ":/res/img/qt.png" ) ),
			MainWindow::tr( "About Qt" ), q, &MainWindow::onAboutQt );

		QFile css( ":/res/css/github-markdown.css" );
		css.open( QIODevice::ReadOnly );
		mdCss = css.readAll();
		css.close();

		QObject::connect( editor->document(), &QTextDocument::modificationChanged,
			saveAction, &QAction::setEnabled );
		QObject::connect( editor->document(), &QTextDocument::modificationChanged,
			q, &MainWindow::setWindowModified );
		QObject::connect( editor, &QPlainTextEdit::textChanged, q, &MainWindow::onTextChanged );
		QObject::connect( editor, &Editor::lineHovered, q, &MainWindow::onLineHovered );
	}

	MainWindow * q = nullptr;
	Editor * editor = nullptr;
	WebView * preview = nullptr;
	PreviewPage * page = nullptr;
	QSplitter * splitter = nullptr;
	HtmlDocument * html = nullptr;
	QAction * newAction = nullptr;
	QAction * openAction = nullptr;
	QAction * saveAction = nullptr;
	QAction * saveAsAction = nullptr;
	bool init = false;
	std::shared_ptr< MD::Document< MD::QStringTrait > > mdDoc;
	QString mdCss;
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

void
MainWindow::openFile( const QString & path )
{
	QFile f( path );
	if( !f.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, windowTitle(),
			tr( "Could not open file %1: %2" ).arg(
				QDir::toNativeSeparators( path ), f.errorString() ) );
		return;
	}

	d->page->setHtml( htmlContent( QFileInfo( path ).absoluteDir().absolutePath() ) );
	d->editor->setDocName( path );
	d->editor->setPlainText( f.readAll() );
	f.close();
	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" )
		.arg( QFileInfo( d->editor->docName() ).fileName() ) );
}

bool
MainWindow::isModified() const
{
	return d->editor->document()->isModified();
}

void
MainWindow::onFileNew()
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to create a new document anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			return;
	}

	d->editor->setDocName( QStringLiteral( "default.md" ) );
	d->page->setHtml( htmlContent( "." ) );
	d->editor->setPlainText( "" );
	d->editor->document()->setModified( false );
	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" ).arg( d->editor->docName() ) );
}

void
MainWindow::onFileOpen()
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to open a new document anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			return;
	}

	QFileDialog dialog( this, tr( "Open MarkDown File" ) );
	dialog.setMimeTypeFilters( { "text/markdown" } );
	dialog.setAcceptMode( QFileDialog::AcceptOpen );

	if( dialog.exec() == QDialog::Accepted )
		openFile( dialog.selectedFiles().constFirst() );
}

void
MainWindow::onFileSave()
{
	if( d->editor->docName() == QStringLiteral( "default.md" ) )
	{
		onFileSaveAs();
		return;
	}

	QFile f( d->editor->docName() );
	if( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QMessageBox::warning( this, windowTitle(),
			tr( "Could not write to file %1: %2" ).arg(
				QDir::toNativeSeparators( d->editor->docName() ), f.errorString() ) );

		return;
	}

	QTextStream str( &f );
	str << d->editor->toPlainText();

	d->editor->document()->setModified( false );

	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" )
		.arg( QFileInfo( d->editor->docName() ).fileName() ) );
}

void
MainWindow::onFileSaveAs()
{
	QFileDialog dialog( this, tr( "Save MarkDown File" ) );
	dialog.setMimeTypeFilters( { "text/markdown" } );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setDefaultSuffix( "md" );

	if( dialog.exec() != QDialog::Accepted )
		return;

	d->editor->setDocName( dialog.selectedFiles().constFirst() );
	d->page->setHtml( htmlContent( QFileInfo( d->editor->docName() )
		.absoluteDir().absolutePath() ) );

	onFileSave();
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to exit anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			e->ignore();
	}
}

QString
MainWindow::htmlContent( const QString & baseUrl ) const
{
	return QStringLiteral( "<!doctype html>\n"
		"<meta charset=\"utf-8\">\n"
		"<head>\n"
		"  <script src=\"qrc:/qtwebchannel/qwebchannel.js\"></script>\n"
		"  <style>\n"
		"  %2\n"
		"  </style>\n"
		"</head>\n"
		"<body>\n"
		"  <base href=\"%1\" />\n"
		"  <div id=\"placeholder\" class=\"markdown-body\"></div>\n"
		"  <script>\n"
		"  'use strict';\n"
		"\n"
		"  var placeholder = document.getElementById('placeholder');\n"
		"\n"
		"  var updateText = function(text) {\n"
		"	  placeholder.innerHTML = text;\n"
		"  }\n"
		"\n"
		"  new QWebChannel(qt.webChannelTransport,\n"
		"	function(channel) {\n"
		"	  var content = channel.objects.content;\n"
		"	  updateText(content.text);\n"
		"	  content.textChanged.connect(updateText);\n"
		"	}\n"
		"  );\n"
		"  </script>\n"
		"</body>\n"
		"</html>" )
			.arg( baseUrl, d->mdCss );
}

void
MainWindow::onTextChanged()
{
	auto md = d->editor->toPlainText();
	QTextStream stream( &md );

	MD::Parser< MD::QStringTrait > parser;
	d->mdDoc = parser.parse( stream, d->editor->docName() );

	d->html->setText( MD::toHtml( d->mdDoc ) );
}

void
MainWindow::onAbout()
{
	QMessageBox::about( this, tr( "About Markdown Editor" ),
		tr( "Markdown Editor.\n\n"
			"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
			"Copyright (c) 2023 Igor Mironchik.\n\n"
			"Licensed under GNU GPL 3.0." ) );
}

void
MainWindow::onAboutQt()
{
	QMessageBox::aboutQt( this );
}

namespace /* anonymous */ {

inline QString
itemType( MD::ItemType t )
{
	switch( t )
	{
		case MD::ItemType::Heading :
			return MainWindow::tr( "Heading" );
		case MD::ItemType::Text :
			return MainWindow::tr( "Text" );
		case MD::ItemType::Paragraph :
			return MainWindow::tr( "Paragraph" );
		case MD::ItemType::LineBreak :
			return MainWindow::tr( "Line Break" );
		case MD::ItemType::Blockquote :
			return MainWindow::tr( "Blockquote" );
		case MD::ItemType::ListItem :
			return MainWindow::tr( "List Item" );
		case MD::ItemType::List :
			return MainWindow::tr( "List" );
		case MD::ItemType::Link :
			return MainWindow::tr( "Link" );
		case MD::ItemType::Image :
			return MainWindow::tr( "Image" );
		case MD::ItemType::Code :
			return MainWindow::tr( "Code" );
		case MD::ItemType::TableCell :
			return MainWindow::tr( "Table Cell" );
		case MD::ItemType::TableRow :
			return MainWindow::tr( "Table Row" );
		case MD::ItemType::Table :
			return MainWindow::tr( "Table" );
		case MD::ItemType::FootnoteRef :
			return MainWindow::tr( "Footnote Reference" );
		case MD::ItemType::Footnote :
			return MainWindow::tr( "Footnote" );
		case MD::ItemType::Document :
			return MainWindow::tr( "Document" );
		case MD::ItemType::PageBreak :
			return MainWindow::tr( "Page Break" );
		case MD::ItemType::Anchor :
			return MainWindow::tr( "Anchor" );
		case MD::ItemType::HorizontalLine :
			return MainWindow::tr( "Horizontal Line" );
		case MD::ItemType::RawHtml :
			return MainWindow::tr( "Raw HTML" );
		case MD::ItemType::Math :
			return MainWindow::tr( "LaTeX Math Expression" );
	}

	return QString();
}

} /* namespace anonymous */

void
MainWindow::onLineHovered( int lineNumber, const QPoint & pos )
{
	for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend(); it != last; ++it )
	{
		if( (*it)->startLine() == lineNumber ||
			( (*it)->type() == MD::ItemType::Code && (*it)->startLine() - 1 == lineNumber ) )
		{
			QToolTip::showText( pos, itemType( (*it)->type() ) );
		}
	}
}

} /* namespace MdEditor */
