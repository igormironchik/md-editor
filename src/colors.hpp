
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

#pragma once

// Qt include.
#include <QDialog>
#include <QColor>
#include <QScopedPointer>
#include <QAbstractButton>


namespace MdEditor {

//
// Colors
//

//! Color scheme.
struct Colors {
	QColor textColor = Qt::blue;
	QColor linkColor = Qt::green;
	QColor listColor = Qt::green;
	QColor inlineColor = Qt::black;
	bool enabled = true;
}; // struct Colors

bool operator != ( const Colors & c1, const Colors & c2 );


//
// ColorsDialog
//

struct ColorsDialogPrivate;

//! Colors dialog.
class ColorsDialog
	:	public QDialog
{
	Q_OBJECT

public:
	explicit ColorsDialog( const Colors & cols, QWidget * parent = nullptr );
	~ColorsDialog() override;

	const Colors & colors() const;

private slots:
	void clicked( QAbstractButton * btn );
	void resetDefaults();
	void applyColors();
	void chooseLinkColor();
	void chooseListColor();
	void chooseTextColor();
	void chooseInlineColor();
	void colorsToggled( bool on );

private:
	Q_DISABLE_COPY( ColorsDialog )

	QScopedPointer< ColorsDialogPrivate > d;
}; // class ColorsDialog

} /* namespace MdEditor */
