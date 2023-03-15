
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
#include "gotoline.hpp"
#include "ui_gotoline.h"
#include "editor.hpp"


namespace MdEditor {

//
// GoToLinePrivate
//

struct GoToLinePrivate {
	GoToLinePrivate( Editor * e, GoToLine * parent )
		:	q( parent )
		,	editor( e )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		QObject::connect( ui.line, &QSpinBox::editingFinished,
			q, &GoToLine::onEditingFinished );
	}

	GoToLine * q = nullptr;
	Editor * editor = nullptr;
	Ui::GoToLine ui;
}; // struct FindPrivate


//
// GoToLine
//

GoToLine::GoToLine( Editor * editor, QWidget * parent )
	:	QFrame( parent )
	,	d( new GoToLinePrivate( editor, this ) )
{
	d->initUi();
}

GoToLine::~GoToLine()
{
}

void
GoToLine::setFocus()
{
	d->ui.line->setFocus();
	d->ui.line->setValue( 0 );
	d->ui.line->selectAll();
}

void
GoToLine::onEditingFinished()
{
	d->editor->goToLine( d->ui.line->value() );

	hide();

	emit hidded();
}

} /* namespace MdEditor */
