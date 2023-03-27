
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
#include "gotoline.hpp"
#include "fontdlg.hpp"
#include "cfg.hpp"

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
#include <QApplication>
#include <QDockWidget>
#include <QTreeWidget>

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/parser.hpp>
#include <md4qt/html.hpp>

// cfgfile include.
#include <cfgfile/all.hpp>


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
		find = new Find( q, editor, ew );
		gotoline = new GoToLine( q, editor, ew );
		v->addWidget( editor );
		v->addWidget( gotoline );
		v->addWidget( find );
		preview = new WebView( w );
		find->hide();
		gotoline->hide();

		splitter->addWidget( ew );
		splitter->addWidget( preview );

		l->addWidget( splitter );

		q->setCentralWidget( w );
		q->setFocusPolicy( Qt::ClickFocus );
		w->setFocusPolicy( Qt::ClickFocus );

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
		loadAllAction = fileMenu->addAction( MainWindow::tr( "Load All Linked Files..." ),
			MainWindow::tr( "Ctrl+R" ), q, &MainWindow::loadAllLinkedFiles );
		loadAllAction->setEnabled( false );
		fileMenu->addSeparator();
		fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/application-exit.png" ) ),
			MainWindow::tr( "Quit" ), MainWindow::tr( "Ctrl+Q" ), q, &QWidget::close );

		editMenuAction = q->menuBar()->addAction( MainWindow::tr( "&Edit" ) );
		toggleFindAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/edit-find.png" ) ),
			MainWindow::tr( "Find/Replace" ), q );
		toggleFindAction->setShortcut( MainWindow::tr( "Ctrl+F" ) );
		q->addAction( toggleFindAction );

		toggleGoToLineAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/go-next-use.png" ) ),
			MainWindow::tr( "Go to Line" ), q );
		toggleGoToLineAction->setShortcut( MainWindow::tr( "Ctrl+L" ) );
		q->addAction( toggleGoToLineAction );

		settingsMenu = q->menuBar()->addMenu( MainWindow::tr( "&Settings" ) );
		auto toggleLineNumbersAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/view-table-of-contents-ltr.png" ) ),
			MainWindow::tr( "Show Line Numbers" ), q );
		toggleLineNumbersAction->setCheckable( true );
		toggleLineNumbersAction->setShortcut( MainWindow::tr( "Alt+L" ) );
		toggleLineNumbersAction->setChecked( true );
		settingsMenu->addAction( toggleLineNumbersAction );

		auto toggleUnprintableCharacters = new QAction(
			QIcon( QStringLiteral( ":/res/img/character-set.png" ) ),
			MainWindow::tr( "Show Tabs/Spaces" ), q );
		toggleUnprintableCharacters->setCheckable( true );
		toggleUnprintableCharacters->setShortcut( MainWindow::tr( "Alt+T" ) );
		toggleUnprintableCharacters->setChecked( true );
		settingsMenu->addAction( toggleUnprintableCharacters );

		settingsMenu->addSeparator();

		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/format-font-size-less.png" ) ),
			MainWindow::tr( "Decrease Font Size" ), MainWindow::tr( "Ctrl+-" ),
			q, &MainWindow::onLessFontSize );
		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/format-font-size-more.png" ) ),
			MainWindow::tr( "Increase Font Size" ), MainWindow::tr( "Ctrl+=" ),
			q, &MainWindow::onMoreFontSize );

		settingsMenu->addSeparator();

		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/preferences-desktop-font.png" ) ),
			MainWindow::tr( "Font..." ),
			q, &MainWindow::onChooseFont );


		auto helpMenu = q->menuBar()->addMenu( MainWindow::tr( "&Help" ) );
		helpMenu->addAction( QIcon( QStringLiteral( ":/res/img/icon_24x24.png" ) ),
			MainWindow::tr( "About" ), q, &MainWindow::onAbout );
		helpMenu->addAction( QIcon( QStringLiteral( ":/res/img/qt.png" ) ),
			MainWindow::tr( "About Qt" ), q, &MainWindow::onAboutQt );

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
		QObject::connect( toggleGoToLineAction, &QAction::triggered,
			q, &MainWindow::onGoToLine );
		QObject::connect( page, &QWebEnginePage::linkHovered,
			[this]( const QString & url )
			{
				if( !url.isEmpty() )
					this->q->statusBar()->showMessage( url );
				else
					this->q->statusBar()->clearMessage();
			} );
		QObject::connect( editor, &QPlainTextEdit::cursorPositionChanged,
			q, &MainWindow::onCursorPositionChanged );

		q->readCfg();

		q->onCursorPositionChanged();

		editor->setFocus();
	}

	MainWindow * q = nullptr;
	Editor * editor = nullptr;
	WebView * preview = nullptr;
	PreviewPage * page = nullptr;
	QSplitter * splitter = nullptr;
	HtmlDocument * html = nullptr;
	Find * find = nullptr;
	GoToLine * gotoline = nullptr;
	QAction * newAction = nullptr;
	QAction * openAction = nullptr;
	QAction * saveAction = nullptr;
	QAction * saveAsAction = nullptr;
	QAction * toggleFindAction = nullptr;
	QAction * toggleGoToLineAction = nullptr;
	QAction * editMenuAction = nullptr;
	QAction * loadAllAction = nullptr;
	QMenu * standardEditMenu = nullptr;
	QMenu * settingsMenu = nullptr;
	QDockWidget * fileTreeDock = nullptr;
	QTreeWidget * fileTree = nullptr;
	bool init = false;
	bool loadAllFlag = false;
	bool previewMode = false;
	std::shared_ptr< MD::Document< MD::QStringTrait > > mdDoc;
	QString baseUrl;
	QString rootFilePath;
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
	if( d->standardEditMenu )
		d->standardEditMenu->deleteLater();

	d->standardEditMenu = nullptr;
}

void
MainWindow::resizeEvent( QResizeEvent * e )
{
	if( !d->init )
	{
		d->init = true;

		auto w = centralWidget()->width() / 2;

		if( !d->previewMode )
			d->splitter->setSizes( { w, w } );
		else
			d->splitter->setSizes( { 0, centralWidget()->width() } );
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
	setWindowTitle( tr( "%1[*] - Markdown Editor%2" )
		.arg( QFileInfo( d->editor->docName() ).fileName(),
			d->previewMode ? tr( " [Preview Mode]" ) : QString() ) );
	d->editor->setFocus();
	d->editor->document()->clearUndoRedoStacks();
	onCursorPositionChanged();
	d->loadAllAction->setEnabled( true );
	d->rootFilePath = path;
	closeAllLinkedFiles();
}

void
MainWindow::openInPreviewMode( bool loadAllLinked )
{
	d->previewMode = true;
	d->loadAllFlag = loadAllLinked;

	if( d->loadAllFlag )
	{
		readAllLinked();

		d->loadAllAction->setText( tr( "Show Only Current File..." ) );
	}
	else
		onTextChanged();

	d->settingsMenu->menuAction()->setVisible( false );
	d->editMenuAction->setVisible( false );
	d->saveAction->setVisible( false );
	d->saveAction->setEnabled( false );
	d->saveAsAction->setVisible( false );
	d->saveAsAction->setEnabled( false );
	d->newAction->setVisible( false );
	d->newAction->setEnabled( false );
	d->editor->setVisible( false );
	d->splitter->handle( 1 )->setEnabled( false );
	d->splitter->handle( 1 )->setVisible( false );

	setWindowTitle( tr( "%1[*] - Markdown Editor [Preview Mode]" )
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
	d->editor->document()->clearUndoRedoStacks();
	setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" ).arg( d->editor->docName() ) );
	d->baseUrl = QString( "file:%1/" ).arg(
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
	d->page->setHtml( htmlContent(), d->baseUrl );
	onCursorPositionChanged();
	d->loadAllAction->setEnabled( false );
	d->rootFilePath.clear();
	closeAllLinkedFiles();
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

	readAllLinked();
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
	d->rootFilePath = d->editor->docName();

	onFileSave();

	d->page->setHtml( htmlContent(), d->baseUrl );

	closeAllLinkedFiles();
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

				if( d->gotoline->isVisible() )
					d->gotoline->hide();
				else if( d->find->isVisible() )
					d->find->hide();

				onToolHide();

				return true;
			}
		}
			break;

		default :
			break;
	}

	return QMainWindow::event( event );
}

void
MainWindow::onToolHide()
{
	if( !d->find->isVisible() && ! d->gotoline->isVisible() )
	{
		d->editor->setFocus();
		d->editor->clearHighlighting();
	}
	else if( d->find->isVisible() && ! d->gotoline->isVisible() )
		d->find->setFocusOnFind();
	else if( d->gotoline->isVisible() && ! d->find->isVisible() )
		d->gotoline->setFocus();
}

QString
MainWindow::htmlContent() const
{
	return QStringLiteral( "<!doctype html>\n"
		"<meta charset=\"utf-8\">\n"
		"<head>\n"
		"  <script src=\"qrc:/qtwebchannel/qwebchannel.js\"></script>\n"
		"  <link rel=\"stylesheet\" href=\"qrc:/res/css/default.min.css\">\n"
		"  <link rel=\"stylesheet\" href=\"qrc:/res/css/github-markdown.css\">\n"
		"  <script src=\"qrc:/res/highlight.min.js\"></script>\n"
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
		"     hljs.highlightAll();\n"
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
		"</html>" );
}

void
MainWindow::onTextChanged()
{
	if( !d->loadAllFlag )
	{
		auto md = d->editor->toPlainText();
		QTextStream stream( &md );

		MD::Parser< MD::QStringTrait > parser;

		d->mdDoc = parser.parse( stream, d->editor->docName() );

		d->html->setText( MD::toHtml( d->mdDoc, false ) );
	}
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
		QString fileName;

		for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend(); it != last; ++it )
		{
			if( (*it)->type() == MD::ItemType::Anchor )
				fileName = static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() )->label();
			else if( d->editor->docName() == fileName )
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
}

void
MainWindow::onFind( bool )
{
	if( !d->find->isVisible() )
		d->find->show();

	if( !d->editor->textCursor().selection().isEmpty() )
		d->find->setFindText( d->editor->textCursor().selection().toPlainText() );
	else
	{
		d->editor->highlightCurrent();
		d->find->setFocusOnFind();
	}
}

void
MainWindow::onGoToLine( bool )
{
	if( !d->gotoline->isVisible() )
		d->gotoline->show();

	d->gotoline->setFocus();
}

void
MainWindow::onChooseFont()
{
	FontDlg dlg( d->editor->font(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->editor->setFont( dlg.font() );

		saveCfg();
	}
}

static const QString c_appCfgFileName = QStringLiteral( "md-editor.cfg" );

void
MainWindow::saveCfg() const
{
	QFile file( QApplication::applicationDirPath() + QDir::separator() + c_appCfgFileName );

	if( file.open( QIODevice::WriteOnly ) )
	{
		try {
			const auto f = d->editor->font();

			Cfg cfg;
			cfg.set_font( f.family() );
			cfg.set_fontSize( f.pointSize() );

			tag_Cfg< cfgfile::qstring_trait_t > tag( cfg );

			QTextStream stream( &file );

			cfgfile::write_cfgfile( tag, stream );

			file.close();
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & )
		{
			file.close();
		}
	}
}

void
MainWindow::readCfg()
{
	QFile file( QApplication::applicationDirPath() + QDir::separator() + c_appCfgFileName );

	if( file.open( QIODevice::ReadOnly ) )
	{
		try {
			tag_Cfg< cfgfile::qstring_trait_t > tag;

			QTextStream stream( &file );

			cfgfile::read_cfgfile( tag, stream, c_appCfgFileName );

			file.close();

			const auto cfg = tag.get_cfg();

			if( !cfg.font().isEmpty() && cfg.fontSize() != -1 )
			{
				const QFont f( cfg.font(), cfg.fontSize() );

				d->editor->setFont( f );
			}
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & )
		{
			file.close();
		}
	}
}

void
MainWindow::onLessFontSize()
{
	auto f = d->editor->font();

	if( f.pointSize() > 5 )
	{
		f.setPointSize( f.pointSize() - 1 );

		d->editor->setFont( f );

		saveCfg();
	}
}

void
MainWindow::onMoreFontSize()
{
	auto f = d->editor->font();

	if( f.pointSize() < 66 )
	{
		f.setPointSize( f.pointSize() + 1 );

		d->editor->setFont( f );

		saveCfg();
	}
}

void
MainWindow::onCursorPositionChanged()
{
	if( d->standardEditMenu )
	{
		d->standardEditMenu->deleteLater();
		d->standardEditMenu = nullptr;
	}

	d->standardEditMenu = d->editor->createStandardContextMenu(
		d->editor->cursorRect().center() );

	d->standardEditMenu->addSeparator();

	d->standardEditMenu->addAction( d->toggleFindAction );
	d->standardEditMenu->addAction( d->toggleGoToLineAction );

	d->editMenuAction->setMenu( d->standardEditMenu );

	connect( d->standardEditMenu, &QMenu::triggered,
		this, &MainWindow::onEditMenuActionTriggered );
}

void
MainWindow::onEditMenuActionTriggered( QAction * action )
{
	if( action != d->toggleFindAction && action != d->toggleGoToLineAction )
		d->editor->setFocus();
}

namespace {

struct Node {
	QVector< QString > keys;
	QVector< QPair< QSharedPointer< Node >, QTreeWidgetItem* > > children;
	QTreeWidgetItem * self = nullptr;
};

}

void
MainWindow::loadAllLinkedFiles()
{
	if( isModified() && !d->previewMode )
	{
		QMessageBox::information( this, windowTitle(),
			tr( "You have unsaved changes. Please save document first." ) );

		d->editor->setFocus();

		return;
	}

	if( d->fileTreeDock )
	{
		closeAllLinkedFiles();

		return;
	}

	if( d->loadAllFlag && d->previewMode )
	{
		d->loadAllAction->setText( tr( "Load All Linked Files..." ) );

		d->loadAllFlag = false;

		onTextChanged();

		return;
	}

	d->loadAllFlag = true;

	readAllLinked();

	if( d->previewMode )
		d->loadAllAction->setText( tr( "Show Only Current File..." ) );

	if( !d->previewMode )
	{
		if( !d->fileTreeDock )
		{
			d->fileTreeDock = new QDockWidget( tr( "Navigation" ), this );
			d->fileTreeDock->setFeatures( QDockWidget::NoDockWidgetFeatures );
		}

		if( !d->fileTree )
		{
			d->fileTree = new QTreeWidget( d->fileTreeDock );
			d->fileTreeDock->setWidget( d->fileTree );
			d->fileTree->setHeaderHidden( true );
		}

		const auto rootFolder = QFileInfo( d->rootFilePath ).absolutePath() + QStringLiteral( "/" );

		Node root;

		for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend(); it != last; ++it )
		{
			if( (*it)->type() == MD::ItemType::Anchor )
			{
				const auto fullFileName =
					static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() )->label();

				const auto fileName = fullFileName.startsWith( rootFolder ) ?
					fullFileName.sliced( rootFolder.size() ) : fullFileName;

				const auto parts = fileName.split( QStringLiteral( "/" ) );

				Node * current = &root;

				for( qsizetype i = 0; i < parts.size(); ++i )
				{
					const QString f = parts.at( i ).isEmpty() ? QStringLiteral( "/" ) : parts.at( i );

					if( i == parts.size() - 1 )
					{
						if( !current->keys.contains( f ) )
						{
							auto tmp = QSharedPointer< Node >::create();
							auto item = new QTreeWidgetItem( current->self );
							item->setIcon( 0, QIcon( ":/res/img/icon_16x16.png" ) );
							item->setData( 0, Qt::UserRole, fullFileName );
							tmp->self = item;
							item->setText( 0, f );
							current->children.push_back( { tmp, item } );
							current->keys.push_back( f );
							current = tmp.get();
						}
					}
					else
					{
						if( !current->keys.contains( f ) )
						{
							auto tmp = QSharedPointer< Node >::create();
							auto item = new QTreeWidgetItem( current->self );
							item->setIcon( 0, QIcon( ":/res/img/folder-yellow.png" ) );
							tmp->self = item;
							item->setText( 0, f );
							current->children.push_back( { tmp, item } );
							current->keys.push_back( f );
							current = tmp.get();
						}
						else
							current = current->children.at( current->keys.indexOf( f ) ).first.get();
					}
				}
			}
		}

		if( root.children.size() > 1 )
		{
			for( auto it = root.children.cbegin(), last = root.children.cend(); it != last; ++it )
				d->fileTree->addTopLevelItem( it->second );

			connect( d->fileTree, &QTreeWidget::itemDoubleClicked,
				this, &MainWindow::onNavigationDoubleClicked );

			addDockWidget( Qt::LeftDockWidgetArea, d->fileTreeDock );

			d->loadAllAction->setText( tr( "Show Only Current File..." ) );

			QMessageBox::information( this, windowTitle(),
				tr( "HTML preview is ready. Modifications in files will not update "
					"HTML preview till you save changes." ) );
		}
		else
		{
			closeAllLinkedFiles();

			QMessageBox::information( this, windowTitle(),
				tr( "This document doesn't have linked documents." ) );
		}

		d->editor->setFocus();
	}
}

void
MainWindow::closeAllLinkedFiles()
{
	d->loadAllFlag = false;

	d->loadAllAction->setText( tr( "Load All Linked Files..." ) );

	if( d->fileTreeDock )
	{
		removeDockWidget( d->fileTreeDock );
		d->fileTreeDock->deleteLater();
	}

	d->fileTree = nullptr;
	d->fileTreeDock = nullptr;

	d->editor->setFocus();

	onTextChanged();
}

void
MainWindow::readAllLinked()
{
	if( d->loadAllFlag )
	{
		MD::Parser< MD::QStringTrait > parser;

		d->mdDoc = parser.parse( d->rootFilePath, true,
			{ QStringLiteral( "md" ), QStringLiteral( "mkd" ), QStringLiteral( "markdown" ) } )	;

		d->html->setText( MD::toHtml( d->mdDoc, false ) );
	}
}

void
MainWindow::onNavigationDoubleClicked( QTreeWidgetItem * item, int )
{
	const auto path = item->data( 0, Qt::UserRole ).toString();

	if( !path.isEmpty() )
	{
		if( isModified() )
		{
			QMessageBox::information( this, windowTitle(),
				tr( "You have unsaved changes. Please save document first." ) );

			d->editor->setFocus();

			return;
		}

		QFile f( path );
		if( !f.open( QIODevice::ReadOnly ) )
		{
			QMessageBox::warning( this, windowTitle(),
				tr( "Could not open file %1: %2" ).arg(
					QDir::toNativeSeparators( path ), f.errorString() ) );
			return;
		}

		d->editor->setDocName( path );
		d->editor->setPlainText( f.readAll() );
		f.close();

		setWindowTitle( MainWindow::tr( "%1[*] - Markdown Editor" )
			.arg( QFileInfo( d->editor->docName() ).fileName() ) );

		d->editor->document()->clearUndoRedoStacks();
		d->editor->setFocus();

		onCursorPositionChanged();
	}
}

} /* namespace MdEditor */
