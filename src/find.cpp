
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


namespace MdEditor {

//
// FindPrivate
//

struct FindPrivate {
	FindPrivate( Find * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		QObject::connect( ui.findEdit, &QLineEdit::textChanged,
			q, &Find::findTextChanged );
		QObject::connect( ui.replaceEdit, &QLineEdit::textChanged,
			q, &Find::replaceTextChanged );
	}

	Find * q = nullptr;
	Ui::Find ui;
}; // struct FindPrivate


//
// Find
//

Find::Find( QWidget * parent )
	:	QFrame( parent )
	,	d( new FindPrivate( this ) )
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
}

void
Find::replaceTextChanged( const QString & str )
{
	d->ui.replaceBtn->setEnabled( !str.isEmpty() );
	d->ui.replaceAllBtn->setEnabled( !str.isEmpty() );
}

} /* namespace MdEditor */
