/*
 * AsmBrowser::extracts text from compiled shader
 * Copyright (C) 2015  R P <digilanti@hotmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASMBROWSER_H
#define ASMBROWSER_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class AsmBrowser : public QWidget
{
    Q_OBJECT

public:
    AsmBrowser(const QStringList &page);

    static void showPage(const QStringList& page, const QString& title);
public slots:
    void saveAsm();

private:
    QTextEdit *textBrowser;
    QPushButton *saveButton;
    QPushButton *closeButton;
};

#endif // ASMBROWSER_H

