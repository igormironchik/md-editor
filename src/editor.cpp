
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
#include "editor.hpp"

// Qt include.
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>


namespace MdEditor {

//
// EditorPrivate
//

struct EditorPrivate {
	EditorPrivate( Editor * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		lineNumberArea = new LineNumberArea( q );

		QObject::connect( q, &Editor::cursorPositionChanged,
			q, &Editor::highlightCurrentLine );

		q->showLineNumbers( true );
		q->setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );
		q->updateLineNumberAreaWidth( 0 );
		q->highlightCurrentLine();
		q->showUnprintableCharacters( true );
	}

	Editor * q = nullptr;
	LineNumberArea * lineNumberArea = nullptr;
	QString docName;
	bool showLineNumberArea = true;
	QList< QTextEdit::ExtraSelection > extraSelections;
	QTextEdit::ExtraSelection currentLine;
}; // struct EditorPrivate


//
// Editor
//

Editor::Editor( QWidget * parent )
	:	QPlainTextEdit( parent )
	,	d( new EditorPrivate( this ) )
{
	d->initUi();
}

Editor::~Editor()
{
}

void
Editor::setDocName( const QString & name )
{
	d->docName = name;
}

const QString &
Editor::docName() const
{
	return d->docName;
}

int
Editor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax( 1, blockCount() );

	while( max >= 10 )
	{
		max /= 10;
		++digits;
	}

	if( digits < 2 )
		digits = 2;

	int space = 3 + fontMetrics().horizontalAdvance( QLatin1Char( '9' ) ) * digits;

	return space;
}

void
Editor::updateLineNumberAreaWidth( int /* newBlockCount */ )
{
	if( d->showLineNumberArea )
		setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
	else
		setViewportMargins( 0, 0, 0, 0 );
}

void
Editor::updateLineNumberArea( const QRect & rect, int dy )
{
    if( dy )
        d->lineNumberArea->scroll( 0, dy );
    else
        d->lineNumberArea->update( 0, rect.y(), d->lineNumberArea->width(), rect.height() );

    if( rect.contains( viewport()->rect() ) )
        updateLineNumberAreaWidth( 0 );
}

void
Editor::resizeEvent( QResizeEvent * e )
{
	QPlainTextEdit::resizeEvent( e );

	QRect cr = contentsRect();
	d->lineNumberArea->setGeometry( QRect( cr.left(), cr.top(),
		lineNumberAreaWidth(), cr.height() ) );
}

void
Editor::highlightCurrentLine()
{
	static const QColor lineColor = QColor( Qt::yellow ).lighter( 180 );

	d->currentLine.format.setBackground( lineColor );
	d->currentLine.format.setProperty( QTextFormat::FullWidthSelection, true );
	d->currentLine.cursor = textCursor();
	d->currentLine.cursor.clearSelection();

	QList< QTextEdit::ExtraSelection > tmp = d->extraSelections;
	tmp.prepend( d->currentLine );

	setExtraSelections( tmp );
}

void
Editor::lineNumberAreaPaintEvent( QPaintEvent * event )
{
	QPainter painter( d->lineNumberArea );
	painter.fillRect( event->rect(), Qt::lightGray );

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
	int bottom = top + qRound( blockBoundingRect( block ).height() );

	while( block.isValid() && top <= event->rect().bottom() )
	{
		if( block.isVisible() && bottom >= event->rect().top() )
		{
			QString number = QString::number( blockNumber + 1 );
			painter.setPen( Qt::black );
			painter.drawText( 0, top, d->lineNumberArea->width(), fontMetrics().height(),
				Qt::AlignRight, number );
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound( blockBoundingRect( block ).height() );
		++blockNumber;
	}
}

int
Editor::lineNumber( const QPoint & p )
{
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
	int bottom = top + qRound( blockBoundingRect( block ).height() );

	while( block.isValid() && top <= p.y() )
	{
		if( block.isVisible() && bottom >= p.y() )
			return blockNumber;

		block = block.next();
		top = bottom;
		bottom = top + qRound( blockBoundingRect( block ).height() );
		++blockNumber;
	}

	return -1;
}

void
LineNumberArea::enterEvent( QEnterEvent * event )
{
	onHover( event->position().toPoint() );

	event->ignore();
}

void
LineNumberArea::mouseMoveEvent( QMouseEvent * event )
{
	onHover( event->position().toPoint() );

	event->ignore();
}

void
LineNumberArea::leaveEvent( QEvent * event )
{
	lineNumber = -1;

	emit hoverLeaved();

	event->ignore();
}

void
LineNumberArea::onHover( const QPoint & p )
{
	const auto ln = codeEditor->lineNumber( p );

	if( ln != lineNumber )
	{
		lineNumber = ln;

		emit lineHovered( lineNumber, mapToGlobal( QPoint( width(), p.y() ) ) );
	}
}

void
Editor::showUnprintableCharacters( bool on )
{
	if( on )
	{
		QTextOption opt;
		opt.setFlags( QTextOption::ShowTabsAndSpaces );

		document()->setDefaultTextOption( opt );
	}
	else
		document()->setDefaultTextOption( {} );

	setTabStopDistance( fontMetrics().horizontalAdvance( QLatin1Char( ' ' ) ) * 4 );
}

void
Editor::showLineNumbers( bool on )
{
	if( on )
	{
		connect( this, &Editor::blockCountChanged,
			this, &Editor::updateLineNumberAreaWidth );
		connect( this, &Editor::updateRequest,
			this, &Editor::updateLineNumberArea );
		connect( d->lineNumberArea, &LineNumberArea::lineHovered,
			this, &Editor::lineHovered );
		connect( d->lineNumberArea, &LineNumberArea::hoverLeaved,
			this, &Editor::hoverLeaved );

		d->lineNumberArea->show();
		d->showLineNumberArea = true;
	}
	else
	{
		disconnect( this, &Editor::blockCountChanged,
			this, &Editor::updateLineNumberAreaWidth );
		disconnect( this, &Editor::updateRequest,
			this, &Editor::updateLineNumberArea );
		disconnect( d->lineNumberArea, &LineNumberArea::lineHovered,
			this, &Editor::lineHovered );
		disconnect( d->lineNumberArea, &LineNumberArea::hoverLeaved,
			this, &Editor::hoverLeaved );

		d->lineNumberArea->hide();
		d->showLineNumberArea = false;
	}

	updateLineNumberAreaWidth( 0 );
}

void
Editor::highlight( const QString & text )
{
	d->extraSelections.clear();

	if( !text.isEmpty() )
	{
		QTextCursor c( document() );

		static const QColor color = QColor( Qt::yellow );

		while( !c.isNull() )
		{
			QTextEdit::ExtraSelection s;

			s.format.setBackground( color );
			s.cursor = document()->find( text, c, QTextDocument::FindCaseSensitively );

			if( !s.cursor.isNull() )
				d->extraSelections.append( s );

			c = s.cursor;
		}
	}

	QList< QTextEdit::ExtraSelection > tmp = d->extraSelections;
	tmp.prepend( d->currentLine );

	setExtraSelections( tmp );
}

void
Editor::onFindNext()
{
	qDebug() << "next";
}

void
Editor::onFindPrev()
{
}

} /* namespace MdEditor */
