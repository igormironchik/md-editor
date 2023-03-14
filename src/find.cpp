
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
#include "find.hpp"
#include "ui_find.h"
#include "editor.hpp"


namespace MdEditor {

//
// FindPrivate
//

struct FindPrivate {
	FindPrivate( Editor * e, Find * parent )
		:	q( parent )
		,	editor( e )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		QObject::connect( ui.findEdit, &QLineEdit::textChanged,
			q, &Find::findTextChanged );
		QObject::connect( ui.replaceEdit, &QLineEdit::textChanged,
			q, &Find::replaceTextChanged );

		auto findPrevAction = new QAction( Find::tr( "Find Previous" ), q );
		findPrevAction->setShortcutContext( Qt::ApplicationShortcut );
		findPrevAction->setShortcut( Find::tr( "Shifgt+F3" ) );
		findPrevAction->setToolTip( Find::tr( "Find Previous <small>Shift+F3</small>" ) );
		ui.findPrevBtn->setDefaultAction( findPrevAction );
		ui.findPrevBtn->setEnabled( false );

		auto findNextAction = new QAction( Find::tr( "Find Next" ), q );
		findNextAction->setShortcutContext( Qt::ApplicationShortcut );
		findNextAction->setShortcut( Find::tr( "F3" ) );
		findNextAction->setToolTip( Find::tr( "Find Next <small>F3</small>" ) );
		ui.findNextBtn->setDefaultAction( findNextAction );
		ui.findNextBtn->setEnabled( false );

		QObject::connect( findPrevAction, &QAction::triggered,
			editor, &Editor::onFindPrev );
		QObject::connect( findNextAction, &QAction::triggered,
			editor, &Editor::onFindNext );
	}

	Find * q = nullptr;
	Editor * editor = nullptr;
	Ui::Find ui;
}; // struct FindPrivate


//
// Find
//

Find::Find( Editor * editor, QWidget * parent )
	:	QFrame( parent )
	,	d( new FindPrivate( editor, this ) )
{
	d->initUi();
}

Find::~Find()
{
}

void
Find::findTextChanged( const QString & str )
{
	d->ui.findNextBtn->setEnabled( !str.isEmpty() );
	d->ui.findPrevBtn->setEnabled( !str.isEmpty() );
	d->ui.findNextBtn->defaultAction()->setEnabled( !str.isEmpty() );
	d->ui.findPrevBtn->defaultAction()->setEnabled( !str.isEmpty() );

	d->editor->highlight( d->ui.findEdit->text() );
}

void
Find::replaceTextChanged( const QString & str )
{
	d->ui.replaceBtn->setEnabled( !str.isEmpty() );
	d->ui.replaceAllBtn->setEnabled( !str.isEmpty() );
}

void
Find::setFindText( const QString & text )
{
	d->ui.findEdit->setText( text );
	d->ui.findEdit->setFocus();
	d->ui.findEdit->selectAll();

	d->editor->highlight( text );
}

} /* namespace MdEditor */
