
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
#include "find.hpp"

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
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QTextDocumentFragment>
#include <QStatusBar>

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
		auto ew = new QWidget( w );
		auto v = new QVBoxLayout( ew );
		v->setContentsMargins( 0, 0, 0, 0 );
		v->setSpacing( 0 );
		editor = new Editor( ew );
		find = new Find( editor, ew );
		v->addWidget( editor );
		v->addWidget( find );
		preview = new WebView( w );
		find->hide();

		splitter->addWidget( ew );
		splitter->addWidget( preview );

		l->addWidget( splitter );

		q->setCentralWidget( w );

		page = new PreviewPage( preview );
		preview->setPage( page );

		html = new HtmlDocument( q );

		auto channel = new QWebChannel( q );
		channel->registerObject( QStringLiteral( "content" ), html );
		page->setWebChannel( channel );

		baseUrl = QString( "file:%1/" ).arg(
			QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
		editor->setDocName( QStringLiteral( "default.md" ) );
		page->setHtml( q->htmlContent(), baseUrl );

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

		auto editMenu = q->menuBar()->addMenu( MainWindow::tr( "&Edit" ) );
		auto toggleFindAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/edit-find.png" ) ),
			MainWindow::tr( "Find/Replace" ), q );
		toggleFindAction->setShortcut( MainWindow::tr( "Ctrl+F" ) );
		editMenu->addAction( toggleFindAction );

		auto settingsMenu = q->menuBar()->addMenu( MainWindow::tr( "&Settings" ) );
		auto toggleLineNumbersAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/view-table-of-contents-ltr.png" ) ),
			MainWindow::tr( "Show Line Numbers" ), q );
		toggleLineNumbersAction->setCheckable( true );
		toggleLineNumbersAction->setShortcut( MainWindow::tr( "Ctrl+L" ) );
		toggleLineNumbersAction->setChecked( true );
		settingsMenu->addAction( toggleLineNumbersAction );

		auto toggleUnprintableCharacters = new QAction(
			QIcon( QStringLiteral( ":/res/img/character-set.png" ) ),
			MainWindow::tr( "Show Tabs/Spaces" ), q );
		toggleUnprintableCharacters->setCheckable( true );
		toggleUnprintableCharacters->setShortcut( MainWindow::tr( "Ctrl+T" ) );
		toggleUnprintableCharacters->setChecked( true );
		settingsMenu->addAction( toggleUnprintableCharacters );

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
		QObject::connect( toggleLineNumbersAction, &QAction::toggled,
			editor, &Editor::showLineNumbers );
		QObject::connect( toggleUnprintableCharacters, &QAction::toggled,
			editor, &Editor::showUnprintableCharacters );
		QObject::connect( toggleFindAction, &QAction::triggered,
			q, &MainWindow::onFind );
		QObject::connect( page, &QWebEnginePage::linkHovered,
			[this]( const QString & url )
			{
				if( !url.isEmpty() )
					this->q->statusBar()->showMessage( url );
				else
					this->q->statusBar()->clearMessage();
			} );
	}

	MainWindow * q = nullptr;
	Editor * editor = nullptr;
	WebView * preview = nullptr;
	PreviewPage * page = nullptr;
	QSplitter * splitter = nullptr;
	HtmlDocument * html = nullptr;
	Find * find = nullptr;
	QAction * newAction = nullptr;
	QAction * openAction = nullptr;
	QAction * saveAction = nullptr;
	QAction * saveAsAction = nullptr;
	bool init = false;
	std::shared_ptr< MD::Document< MD::QStringTrait > > mdDoc;
	QString mdCss;
	QString baseUrl;
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

	d->editor->setDocName( path );
	d->baseUrl = QString( "file:%1/" ).arg( QFileInfo( path ).absoluteDir().absolutePath() );
	d->page->setHtml( htmlContent(), d->baseUrl );

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
	d->editor->setPlainText( "" );
	d->editor->document()->setModified( false );
	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" ).arg( d->editor->docName() ) );
	d->baseUrl = QString( "file:%1/" ).arg(
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
	d->page->setHtml( htmlContent(), d->baseUrl );
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

	QFileDialog dialog( this, tr( "Open Markdown File" ),
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
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
	f.close();

	d->editor->document()->setModified( false );

	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" )
		.arg( QFileInfo( d->editor->docName() ).fileName() ) );
}

void
MainWindow::onFileSaveAs()
{
	QFileDialog dialog( this, tr( "Save Markdown File" ),
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
	dialog.setMimeTypeFilters( { "text/markdown" } );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setDefaultSuffix( "md" );

	if( dialog.exec() != QDialog::Accepted )
		return;

	d->editor->setDocName( dialog.selectedFiles().constFirst() );
	d->baseUrl = QString( "file:%1/" ).arg( QFileInfo( d->editor->docName() )
		.absoluteDir().absolutePath() );

	onFileSave();

	d->page->setHtml( htmlContent(), d->baseUrl );
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

bool
MainWindow::event( QEvent * event )
{
	switch( event->type() )
	{
		case QEvent::ShortcutOverride :
		{
			if( static_cast< QKeyEvent* >( event )->key() == Qt::Key_Escape )
			{
				event->accept();

				d->find->hide();
				d->editor->setFocus();
				d->editor->clearExtraSelections();

				return true;
			}
		}
			break;

		default :
			break;
	}

	return QMainWindow::event( event );
}

QString
MainWindow::htmlContent() const
{
	return QStringLiteral( "<!doctype html>\n"
		"<meta charset=\"utf-8\">\n"
		"<head>\n"
		"  <script src=\"qrc:/qtwebchannel/qwebchannel.js\"></script>\n"
		"  <style>\n"
		"  %1\n"
		"  </style>\n"
		"</head>\n"
		"<body>\n"
		"  <div id=\"placeholder\"></div>\n"
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
			.arg( d->mdCss );
}

void
MainWindow::onTextChanged()
{
	auto md = d->editor->toPlainText();
	QTextStream stream( &md );

	MD::Parser< MD::QStringTrait > parser;
	d->mdDoc = parser.parse( stream, d->editor->docName() );

	d->html->setText( MD::toHtml( d->mdDoc, false ) );
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

inline bool
inRange( long long int start, long long int end, int pos )
{
	return ( pos >= start && pos <= end );
}

} /* namespace anonymous */

void
MainWindow::onLineHovered( int lineNumber, const QPoint & pos )
{
	if( d->mdDoc.get() )
	{
		for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend(); it != last; ++it )
		{
			if( (*it)->type() == MD::ItemType::List || (*it)->type() == MD::ItemType::Footnote )
			{
				bool exit = false;

				auto list = static_cast< MD::List< MD::QStringTrait > * > ( it->get() );

				for( auto lit = list->items().cbegin(), llast = list->items().cend();
					lit != llast; ++lit )
				{
					if( (*lit)->startLine() == lineNumber )
					{
						QToolTip::showText( pos, itemType( (*it)->type() ) );

						exit = true;

						break;
					}
					else
					{
						auto listItem = static_cast< MD::ListItem< MD::QStringTrait > * > ( lit->get() );

						for( auto iit = listItem->items().cbegin(), ilast = listItem->items().cend();
							 iit != ilast; ++iit )
						{
							if( inRange( (*iit)->startLine(), (*iit)->endLine(), lineNumber ) ||
								( (*iit)->type() == MD::ItemType::Code &&
									inRange( (*iit)->startLine() - 1, (*iit)->endLine() + 1, lineNumber ) ) )
							{
								QToolTip::showText( pos, tr( "%1 in %2" )
									.arg( itemType( (*iit)->type() ), itemType( (*it)->type() ) ) );

								exit = true;

								break;
							}
						}
					}
				}

				if( exit )
					break;
			}
			else if( inRange( (*it)->startLine(), (*it)->endLine(), lineNumber ) ||
				( (*it)->type() == MD::ItemType::Code &&
					inRange( (*it)->startLine() - 1, (*it)->endLine() + 1, lineNumber ) ) )
			{
				QToolTip::showText( pos, itemType( (*it)->type() ) );

				break;
			}
		}
	}
}

void
MainWindow::onFind( bool )
{
	if( !d->find->isVisible() )
		d->find->show();

	d->find->setFindText( d->editor->textCursor().selection().toPlainText() );
}

} /* namespace MdEditor */
