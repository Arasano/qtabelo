/**
 * Copyright 2022 naracanto <https://naracanto.github.io>.
 *
 * This file is part of QTabelo <https://github.com/beletalabs/qtabelo>.
 *
 * QTabelo is an open source table editor written in C++ using the
 * Qt framework.
 *
 * QTabelo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * QTabelo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QTabelo.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "document_widget.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QWidget>

#include "rename_dialog.h"


DocumentWidget::DocumentWidget(QWidget *parent)
    : TableDocument(parent), m_modified{false}, m_url{QUrl()}
{
    setAttribute(Qt::WA_DeleteOnClose);

    // Подключаем сигнал изменения содержимого таблицы к флагу modified
    connect(this, &TableDocument::contentChanged, this, &DocumentWidget::setModified);
}


void DocumentWidget::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}


//
// Property: modified
//

bool DocumentWidget::isModified() const
{
    return m_modified;
}


void DocumentWidget::setModified(const bool modified)
{
    if (modified != m_modified) {
        m_modified = modified;
        emit modifiedChanged(modified);
    }
}


void DocumentWidget::resetModified()
{
    m_modified = false;
    emit modifiedChanged(false);
}


void DocumentWidget::initModified()
{
    emit modifiedChanged(m_modified);
}


//
// Property: url
//

QUrl DocumentWidget::url() const
{
    return m_url;
}


void DocumentWidget::setUrl(const QUrl &url)
{
    if (url != m_url) {
        m_url = url;
        emit urlChanged(url);
    }
}


void DocumentWidget::resetUrl()
{
    m_url = QUrl();
    emit urlChanged(QUrl());
}


void DocumentWidget::initUrl()
{
    emit urlChanged(m_url);
}


//
// Document
//

void DocumentWidget::documentCountChanged(const int count)
{
    slotAddTab(count);
}


//
//
//

void DocumentWidget::copyPathToClipboard()
{
    if (!m_url.isEmpty())
        QApplication::clipboard()->setText(m_url.toDisplayString(QUrl::PreferLocalFile));
}


void DocumentWidget::copyFilenameToClipboard()
{
    if (!m_url.fileName().isEmpty())
        QApplication::clipboard()->setText(m_url.fileName());
}


void DocumentWidget::renameFilename()
{
    if (!m_url.isLocalFile())
        return;

    bool ok = false;
    const QString oldFileName = m_url.fileName();
    const QString newFileName = RenameDialog::getFileName(this, tr("Rename file"), tr("New file name"), QLineEdit::Normal, oldFileName, &ok);
    if (!ok || newFileName.isEmpty() || (newFileName == oldFileName))
        return;

    QUrl newUrl = m_url.adjusted(QUrl::RemoveFilename).toString() + newFileName;

    if (!QFile::rename(m_url.toLocalFile(), newUrl.toLocalFile())) {

        QString title = tr("File Already Exists");
        QString text = tr("A file with the name <em>%1</em> already exists!").arg(newUrl.fileName());

        QMessageBox::critical(this, title, text);
        return;
    }

    setUrl(newUrl);
}
