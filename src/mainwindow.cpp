
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
			MainWindow::tr( "Quit" ), q, &QWidget::close );

		QObject::connect( editor->document(), &QTextDocument::modificationChanged,
			saveAction, &QAction::setEnabled );
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
	d->editor->setPlainText( f.readAll() );
	f.close();
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
			tr( "You have unsaved changes. Do you want to create a new document anyway?" ) );

		if( button != QMessageBox::Yes )
			return;
	}

	d->editor->setDocName( QStringLiteral( "default.md" ) );
	d->editor->setPlainText( "" );
	d->editor->document()->setModified( false );
}

void
MainWindow::onFileOpen()
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to open a new document anyway?" ) );

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
	onFileSave();
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to exit anyway?" ) );

		if( button != QMessageBox::Yes )
			e->ignore();
	}
}

} /* namespace MdEditor */
