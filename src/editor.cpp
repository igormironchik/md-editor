
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

		QObject::connect( q, &Editor::blockCountChanged,
			q, &Editor::updateLineNumberAreaWidth );
		QObject::connect( q, &Editor::updateRequest,
			q, &Editor::updateLineNumberArea );
		QObject::connect( q, &Editor::cursorPositionChanged,
			q, &Editor::highlightCurrentLine );

		q->updateLineNumberAreaWidth( 0 );
		q->highlightCurrentLine();
	}

	Editor * q = nullptr;
	LineNumberArea * lineNumberArea = nullptr;
	QString docName;
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
    setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
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
	QList< QTextEdit::ExtraSelection > extraSelections;

	QTextEdit::ExtraSelection selection;

	QColor lineColor = QColor( Qt::yellow ).lighter( 160 );

	selection.format.setBackground( lineColor );
	selection.format.setProperty( QTextFormat::FullWidthSelection, true );
	selection.cursor = textCursor();
	selection.cursor.clearSelection();
	extraSelections.append( selection );

	setExtraSelections( extraSelections );
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

} /* namespace MdEditor */
