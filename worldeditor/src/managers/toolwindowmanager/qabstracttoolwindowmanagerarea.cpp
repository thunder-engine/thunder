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

#include "qabstracttoolwindowmanagerarea.h"
#include "qtoolwindowmanager.h"
#include "private/qtoolwindowmanager_p.h"

/*!
    \class QAbstractToolWindowManagerArea

    \brief The QAbstractToolWindowManagerArea class is the base class for widgets
    used by QToolWindowManager to display one or several tool windows in its layout
    or floating windows.

    To customize appearance and behavior of areas, subclass QAbstractToolWindowManagerArea
    and implement its virtual functions. Also subclass QToolWindowManager and reimplement
    QToolWindowManager::createArea to return new object of created subclass.

    A subclass of QAbstractToolWindowManagerArea should implement displaying one or multiple
    tool windows, ability to switch between them, drag one tool window or entire area out of
    its place using mouse.

    \inmodule QtWidgets

    \since 5.4

 */
/*!
    \fn QAbstractToolWindowManagerArea::toolWindows() const

    Implement this function to return list of tool windows in this area in actual order.
 */
/*!
    \fn QAbstractToolWindowManagerArea::activateToolWindow(QWidget *toolWindow)

    Implement this function to activate (show, raise) \a toolWindow in the area. \a toolWindow
    must be a member of the area.
 */
/*!
    \fn QAbstractToolWindowManagerArea::addToolWindows(const QWidgetList &toolWindows)

    Implement this function to add \a toolWindows to the area. The area may become a parent
    of tool windows to display them. \a toolWindows should appear
    in the end of QAbstractToolWindowManagerArea::toolWindows list immediately after this action.
    However if the area implementation provides a way to change tool windows order and user has
    changed it, QAbstractToolWindowManagerArea::toolWindow order should change accordingly.
 */
/*!
    \fn QAbstractToolWindowManagerArea::removeToolWindow(QWidget *toolWindow)

    Implement this function to remove \a toolWindow from the area. Ownership and visibility
    of \a toolWindow should not be changed, as it's a responsibility of the manager to adjust them.
 */

/*! Creates new area for the \a manager. */
QAbstractToolWindowManagerArea::QAbstractToolWindowManagerArea(QToolWindowManager *manager) :
    QWidget(manager),
    m_manager(manager)
{
}

/*! Returns the manager for this area. */
QToolWindowManager *QAbstractToolWindowManagerArea::manager() const
{
    return m_manager;
}

/*!
 * This virtual function should be reimplemented to return additional state data specific
 * to this area, e.g. current tool window (if applicable). Default implementation returns
 * invalid QVariant.
 */
QVariant QAbstractToolWindowManagerArea::saveState() const
{
    return QVariant();
}

/*!
 * This virtual function should be reimplemented to restore state based on \a state data
 * obtained earlier from QAbstractToolWindowManagerArea::saveState function. Default implementation
 * does nothing.
 */
void QAbstractToolWindowManagerArea::restoreState(const QVariant &state)
{
    Q_UNUSED(state);
}

void QAbstractToolWindowManagerArea::beforeTabButtonChanged(QWidget* toolWindow)
{
  Q_UNUSED(toolWindow);
}

void QAbstractToolWindowManagerArea::tabButtonChanged(QWidget* toolWindow)
{
  Q_UNUSED(toolWindow);
}

/*!
 * When user starts dragging tool windows from the area, it continues to receive mouse events
 * even if the mouse pointer leaves its boundaries. Call this function on every mouse move or
 * release event to update manager's state according to new mouse state.
 *
 * TODO: we should try to implement this internally using event filters.
 */
void QAbstractToolWindowManagerArea::updateDragPosition()
{
    QToolWindowManagerPrivate * const manager_d = m_manager->d_func();
    manager_d->updateDragPosition();
}

/*!
 * Call this function to notify the manager that a drag operation on \a toolWindows has begun.
 * The manager will take control of this process and move \a toolWindows to new position when
 * the drag operation ends successfully.
 */
void QAbstractToolWindowManagerArea::startDrag(const QWidgetList &toolWindows) {
    QToolWindowManagerPrivate * const manager_d = m_manager->d_func();
    manager_d->startDrag(toolWindows);
}
