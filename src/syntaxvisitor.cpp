
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2023-2024 Igor Mironchik

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
#include "syntaxvisitor.hpp"
#include "editor.hpp"
#include "colors.hpp"

// Qt include.
#include <QTextCursor>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QTextBlock>


namespace MdEditor {

//
// SyntaxVisitorPrivate
//

struct SyntaxVisitorPrivate {
	SyntaxVisitorPrivate( Editor * e )
		:	editor( e )
	{
	}

	void clearFormats()
	{
		for( const auto & f : std::as_const( formats ) )
			f.block.layout()->clearFormats();

		formats.clear();
	}

	void applyFormats()
	{
		for( const auto & f : std::as_const( formats ) )
			f.block.layout()->setFormats( f.format );
	}

	void setFormat( const QTextCharFormat & format,
		long long int startLine, long long int startColumn,
		long long int endLine, long long int endColumn )
	{
		for( auto i = startLine; i <= endLine; ++i )
		{
			formats[ i ].block = editor->document()->findBlockByNumber( i );

			QTextLayout::FormatRange r;
			r.format = format;
			r.start = ( i == startLine ? startColumn : 0 );
			r.length = ( i == startLine ?
				( i == endLine ? endColumn - startColumn + 1 :
					formats[ i ].block.length() - startColumn ) :
				( i == endLine ? endColumn + 1 : formats[ i ].block.length() ) );

			formats[ i ].format.push_back( r );
		}
	}

	//! Editor.
	Editor * editor = nullptr;
	//! Document.
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc;
	//! Colors.
	Colors colors;

	struct Format {
		QTextBlock block;
		QList< QTextLayout::FormatRange > format;
	};

	//! Formats.
	QMap< int, Format > formats;
	//! Default font.
	QFont font;
}; // struct SyntaxVisitorPrivate


//
// SyntaxVisitor
//

SyntaxVisitor::SyntaxVisitor( Editor * editor )
	:	d( new SyntaxVisitorPrivate( editor ) )
{
}

SyntaxVisitor::~SyntaxVisitor()
{
}

void
SyntaxVisitor::setFont( const QFont & f )
{
	d->font = f;
}

void
SyntaxVisitor::highlight( std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const Colors & colors )
{
	d->clearFormats();

	d->doc = doc;
	d->colors = colors;

	if( d->doc )
	{
		MD::Visitor< MD::QStringTrait >::process( d->doc );

		for( auto it = d->doc->footnotesMap().cbegin(), last = d->doc->footnotesMap().cend();
			it != last; ++it )
		{
			onFootnote( it->second.get() );
		}
	}

	d->applyFormats();
}

void
SyntaxVisitor::onAddLineEnding()
{
}

void
SyntaxVisitor::onText( MD::Text< MD::QStringTrait > * t )
{
	QTextCharFormat format;
	format.setForeground( d->colors.textColor );
	format.setFont( d->font );

	d->setFormat( format, t->startLine(), t->startColumn(),
		t->endLine(), t->endColumn() );
}

void
SyntaxVisitor::onMath( MD::Math< MD::QStringTrait > * m )
{
}

void
SyntaxVisitor::onLineBreak( MD::LineBreak< MD::QStringTrait > * b )
{
}

void
SyntaxVisitor::onParagraph( MD::Paragraph< MD::QStringTrait > * p, bool wrap )
{
	MD::Visitor< MD::QStringTrait >::onParagraph( p, wrap );
}

void
SyntaxVisitor::onHeading( MD::Heading< MD::QStringTrait > * h )
{
}

void
SyntaxVisitor::onCode( MD::Code< MD::QStringTrait > * c )
{
}

void
SyntaxVisitor::onInlineCode( MD::Code< MD::QStringTrait > * c )
{
}

void
SyntaxVisitor::onBlockquote( MD::Blockquote< MD::QStringTrait > * b )
{
	MD::Visitor< MD::QStringTrait >::onBlockquote( b );
}

void
SyntaxVisitor::onList( MD::List< MD::QStringTrait > * l )
{
	bool first = true;

	for( auto it = l->items().cbegin(), last = l->items().cend(); it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::ListItem )
		{
			onListItem( static_cast< MD::ListItem< MD::QStringTrait >* > ( it->get() ), first );

			first = false;
		}
	}
}

void
SyntaxVisitor::onTable( MD::Table< MD::QStringTrait > * t )
{
	if( !t->isEmpty() )
	{
		int columns = 0;

		for( auto th = (*t->rows().cbegin())->cells().cbegin(),
			last = (*t->rows().cbegin())->cells().cend(); th != last; ++th )
		{
			onTableCell( th->get() );

			++columns;
		}

		for( auto r = std::next( t->rows().cbegin() ), rlast = t->rows().cend(); r != rlast; ++r )
		{
			int i = 0;

			for( auto c = (*r)->cells().cbegin(), clast = (*r)->cells().cend(); c != clast; ++c )
			{
				onTableCell( c->get() );

				++i;

				if( i == columns )
					break;
			}
		}
	}
}

void
SyntaxVisitor::onAnchor( MD::Anchor< MD::QStringTrait > * a )
{
}

void
SyntaxVisitor::onRawHtml( MD::RawHtml< MD::QStringTrait > * h )
{
}

void
SyntaxVisitor::onHorizontalLine( MD::HorizontalLine< MD::QStringTrait > * l )
{
}

void
SyntaxVisitor::onLink( MD::Link< MD::QStringTrait > * l )
{
	QTextCharFormat format;
	format.setForeground( d->colors.linkColor );
	format.setFont( d->font );

	d->setFormat( format, l->startLine(), l->startColumn(),
		l->endLine(), l->endColumn() );

	if( l->p() )
		onParagraph( l->p().get(), true );
}

void
SyntaxVisitor::onImage( MD::Image< MD::QStringTrait > * i )
{
}

void
SyntaxVisitor::onFootnoteRef( MD::FootnoteRef< MD::QStringTrait > * ref )
{
}

void
SyntaxVisitor::onFootnote( MD::Footnote< MD::QStringTrait > * f )
{
	MD::Visitor< MD::QStringTrait >::onFootnote( f );
}

} /* namespace MdEditor */
