
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

#pragma once

// Qt include.
#include <QPlainTextEdit>
#include <QScopedPointer>


namespace MdEditor {

//
// Editor
//

struct EditorPrivate;

//! Markdown text editor.
class Editor
	:	public QPlainTextEdit
{
	Q_OBJECT

public:
	explicit Editor( QWidget * parent );
	~Editor() override;

	void setDocName( const QString & name );
	const QString & docName() const;

private:
	Q_DISABLE_COPY( Editor )

	QScopedPointer< EditorPrivate > d;
}; // class Editor

} /* namespace MdEditor */
