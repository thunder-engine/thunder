/****************************************************************************
**
** Copyright (C) 2014 Pavel Strakhov <ri@idzaaus.org>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QBoxLayout>
#include <qevent.h>
#include <qapplication.h>
#include <qsplitter.h>
#include "qtoolwindowmanager.h"
#include "qabstracttoolwindowmanagerarea.h"
#include "private/qtoolwindowmanager_p.h"
#include "private/qtoolwindowmanagerwrapper_p.h"

QToolWindowManagerWrapper::QToolWindowManagerWrapper(QToolWindowManager *manager) :
    QWidget(manager),
    m_manager(manager)
{
    setWindowFlags(windowFlags() | Qt::Window);
    setWindowTitle(QLatin1String(" "));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QToolWindowManagerPrivate * const manager_d = m_manager->d_func();
    manager_d->m_wrappers << this;
}

QToolWindowManagerWrapper::~QToolWindowManagerWrapper()
{
    QToolWindowManagerPrivate * const manager_d = m_manager->d_func();
    manager_d->m_wrappers.removeOne(this);
}

void QToolWindowManagerWrapper::closeEvent(QCloseEvent *)
{
    QList<QWidget*> toolWindows;
    foreach (QAbstractToolWindowManagerArea *tabWidget, findChildren<QAbstractToolWindowManagerArea *>())
        toolWindows << tabWidget->toolWindows();
    m_manager->moveToolWindows(toolWindows, QToolWindowManager::NoArea);
}

QVariantMap QToolWindowManagerWrapper::saveState() const
{
    if (layout()->count() > 1) {
        qWarning("too many children for wrapper");
        return QVariantMap();
    }
    if (isWindow() && layout()->count() == 0) {
        qWarning("empty top level wrapper");
        return QVariantMap();
    }
    QVariantMap result;
    result[QLatin1String("geometry")] = saveGeometry();
    QSplitter *splitter = findChild<QSplitter*>();
    QToolWindowManagerPrivate * const manager_d = m_manager->d_func();
    if (splitter) {
        result[QLatin1String("splitter")] = manager_d->saveSplitterState(splitter);
    } else {
        QAbstractToolWindowManagerArea *area = findChild<QAbstractToolWindowManagerArea *>();
        if (area) {
            result[QLatin1String("area")] = manager_d->saveAreaState(area);
        } else if (layout()->count() > 0) {
            qWarning("unknown child");
            return QVariantMap();
        }
    }
    return result;
}

void QToolWindowManagerWrapper::restoreState(const QVariantMap &data)
{
    restoreGeometry(data[QLatin1String("geometry")].toByteArray());
    if (layout()->count() > 0) {
        for(int i = 0; i < layout()->count(); i++) {
            QSplitter *invalidSplitter = qobject_cast<QSplitter *>(layout()->itemAt(i)->widget());
            if(invalidSplitter) {
                invalidSplitter->hide();
                invalidSplitter->setParent(nullptr);
                invalidSplitter->deleteLater();
            }
        }
    }
    QToolWindowManagerPrivate *const manager_d = m_manager->d_func();
    if (data.contains(QLatin1String("splitter"))) {
        layout()->addWidget(manager_d->restoreSplitterState(data[QLatin1String("splitter")].toMap()));
    } else if (data.contains(QLatin1String("area"))) {
        layout()->addWidget(manager_d->restoreAreaState(data[QLatin1String("area")].toMap()));
    }
}
