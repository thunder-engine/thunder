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

#ifndef QTOOLWINDOWMANAGERWRAPPER_P_H
#define QTOOLWINDOWMANAGERWRAPPER_P_H

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/qwidget.h>
#else
#include <QtGui/qwidget.h>
#define Q_DECL_OVERRIDE
#endif

#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLWINDOWMANAGER

class QToolWindowManager;

/*!
 * The ToolWindowManagerWrapper class is used by ToolWindowManager to wrap its content.
 * One wrapper is a direct child of the manager and contains tool windows that are inside its window.
 * All other wrappers are top level floating windows that contain detached tool windows.
 *
 */
class QToolWindowManagerWrapper : public QWidget
{
    Q_OBJECT
public:
    explicit QToolWindowManagerWrapper(QToolWindowManager *manager);
    virtual ~QToolWindowManagerWrapper();

protected:
    // Reimplemented to register hiding of contained tool windows when user closes the floating window.
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(QToolWindowManagerWrapper)
    QToolWindowManager *m_manager;

    //dump content's layout to variable
    QVariantMap saveState() const;

    //construct layout based on given dump
    void restoreState(const QVariantMap& data);

    friend class QToolWindowManager;
};

#endif // QT_NO_TOOLWINDOWMANAGER

QT_END_NAMESPACE

#endif // QTOOLWINDOWMANAGERWRAPPER_P_H
