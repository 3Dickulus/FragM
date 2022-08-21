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

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>

#include "asmbrowser.h"

AsmBrowser::AsmBrowser(const QStringList &page) : QWidget(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_GroupLeader);

    textBrowser = new QTextEdit;
    saveButton = new QPushButton(tr("&Save"));
    closeButton = new QPushButton(tr("Close"));

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(textBrowser);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveAsm()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    textBrowser->setText(page.join("\n"));
}

void AsmBrowser::showPage(const QStringList &page, const QString &title)
{
    QStringList newPage;

    QList<QString>::const_iterator i = page.begin();
    int n = 0;
    while (i != page.end()) {
        newPage << QString("%1\t").arg(n) + page.at(n);
        ++i;
        ++n;
    }

    auto *browser = new AsmBrowser(newPage);
    browser->setWindowTitle(title);
    browser->resize(500, 400);
    browser->show();
}

void AsmBrowser::saveAsm()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"));
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(
            this, tr("Fragmentarium"),
            tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << textBrowser->toPlainText();
}
