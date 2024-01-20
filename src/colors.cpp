
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
#include "colors.hpp"
#include "ui_colors.h"

// Qt include.
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>


namespace MdEditor {

bool operator != ( const Colors & c1, const Colors & c2 )
{
	return ( c1.enabled != c2.enabled || c1.inlineColor != c2.inlineColor ||
		c1.linkColor != c2.linkColor || c1.listColor != c2.listColor ||
		c1.textColor != c2.textColor );
}

//
// ColorsDialogPrivate
//

struct ColorsDialogPrivate {
	Colors colors;
	Ui::ColorsDialog ui;
}; // struct ColorsDialogPrivate

//
// ColorsDialog
//

ColorsDialog::ColorsDialog( const Colors & cols, QWidget * parent )
	:	QDialog( parent )
	,	d( new ColorsDialogPrivate )
{
	d->colors = cols;

	d->ui.setupUi( this );

	applyColors();

	connect( d->ui.buttonBox, &QDialogButtonBox::clicked,
		this, &ColorsDialog::clicked );
	connect( d->ui.linkColor, &ColorWidget::clicked,
		this, &ColorsDialog::chooseLinkColor );
	connect( d->ui.listColor, &ColorWidget::clicked,
		this, &ColorsDialog::chooseListColor );
	connect( d->ui.textColor, &ColorWidget::clicked,
		this, &ColorsDialog::chooseTextColor );
	connect( d->ui.inlineColor, &ColorWidget::clicked,
		this, &ColorsDialog::chooseInlineColor );
	connect( d->ui.colors, &QGroupBox::toggled,
		this, &ColorsDialog::colorsToggled );
}

ColorsDialog::~ColorsDialog()
{
}

const Colors &
ColorsDialog::colors() const
{
	return d->colors;
}

void
ColorsDialog::clicked( QAbstractButton * btn )
{
	if( static_cast< QAbstractButton* > ( d->ui.buttonBox->button(
		QDialogButtonBox::RestoreDefaults ) ) == btn )
			resetDefaults();
}

void
ColorsDialog::resetDefaults()
{
	d->colors = {};

	applyColors();
}

void
ColorsDialog::applyColors()
{
	d->ui.colors->setChecked( d->colors.enabled );

	d->ui.inlineColor->setColor( d->colors.inlineColor );
	d->ui.linkColor->setColor( d->colors.linkColor );
	d->ui.listColor->setColor( d->colors.listColor );
	d->ui.textColor->setColor( d->colors.textColor );
}

void
ColorsDialog::chooseLinkColor()
{
	QColorDialog dlg( d->ui.linkColor->color(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->ui.linkColor->setColor( dlg.currentColor() );
		d->colors.linkColor = dlg.currentColor();
	}
}

void
ColorsDialog::chooseListColor()
{
	QColorDialog dlg( d->ui.listColor->color(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->ui.listColor->setColor( dlg.currentColor() );
		d->colors.listColor = dlg.currentColor();
	}
}

void
ColorsDialog::chooseTextColor()
{
	QColorDialog dlg( d->ui.textColor->color(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->ui.textColor->setColor( dlg.currentColor() );
		d->colors.textColor = dlg.currentColor();
	}
}

void
ColorsDialog::chooseInlineColor()
{
	QColorDialog dlg( d->ui.inlineColor->color(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->ui.inlineColor->setColor( dlg.currentColor() );
		d->colors.inlineColor = dlg.currentColor();
	}
}

void
ColorsDialog::colorsToggled( bool on )
{
	d->colors.enabled = on;
}

} /* namespace MdEditor */
