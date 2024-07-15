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

#include <QApplication>
#include <QEvent>
#include <QTabBar>
#include <QBoxLayout>
#include <QTabWidget>
#include "qtoolwindowmanager.h"
#include "private/qtoolwindowmanager_p.h"
#include "private/qtoolwindowmanagerarea_p.h"

QToolWindowManagerArea::QToolWindowManagerArea(QToolWindowManager *manager) :
    QAbstractToolWindowManagerArea(manager)
{
    d_ptr = new QToolWindowManagerAreaPrivate();
    d_ptr->q_ptr = this;
    Q_D(QToolWindowManagerArea);
    d->m_manager = manager;
    d->m_d_manager = d->m_manager->d_func();
    d->m_tabWidget = new QTabWidget();
    d->slots_object.d = d;
    connect(d->m_tabWidget, SIGNAL(tabCloseRequested(int)),
            &(d->slots_object), SLOT(tabCloseRequested(int)));
    connect(d->m_tabWidget, &QTabWidget::currentChanged, &(d->m_d_manager->slots_object), &QToolWindowManagerPrivateSlots::onCurrentChanged);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->m_tabWidget);
    d->m_dragCanStart = false;
    d->m_tabDragCanStart = false;
    d->m_tabWidget->setMovable(true);
    d->m_tabWidget->setTabsClosable(d->m_d_manager->m_tabsClosable);
    connect(d->m_manager, SIGNAL(tabsClosableChanged(bool)),
            this, SLOT(managerTabsClosableChanged(bool)));
    //d->m_tabWidget->setDocumentMode(true);
    d->m_tabWidget->tabBar()->installEventFilter(this);
}

QToolWindowManagerArea::~QToolWindowManagerArea()
{
    delete d_ptr;
}

void QToolWindowManagerArea::mousePressEvent(QMouseEvent *)
{
    Q_D(QToolWindowManagerArea);
    if (qApp->mouseButtons() == Qt::LeftButton)
        d->m_dragCanStart = true;
}

void QToolWindowManagerArea::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QToolWindowManagerArea);
    d->m_dragCanStart = false;
    updateDragPosition();
}

void QToolWindowManagerArea::mouseMoveEvent(QMouseEvent *)
{
    Q_D(QToolWindowManagerArea);
    d->check_mouse_move();
}

bool QToolWindowManagerArea::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QToolWindowManagerArea);
    if (object == d->m_tabWidget->tabBar()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (qApp->mouseButtons() == Qt::LeftButton) {
                // can start tab drag only if mouse is at some tab, not at empty tabbar space
                if (d->m_tabWidget->tabBar()->tabAt(static_cast<QMouseEvent*>(event)->pos()) >= 0 )
                    d->m_tabDragCanStart = true;
                else
                    d->m_dragCanStart = true;
            }
            break;
        case QEvent::MouseButtonRelease:
            d->m_tabDragCanStart = false;
            d->m_dragCanStart = false;
            updateDragPosition();
            break;
        case QEvent::MouseMove:
            updateDragPosition();
            if (d->m_tabDragCanStart) {
                if (d->m_tabWidget->tabBar()->rect().contains(static_cast<QMouseEvent*>(event)->pos()))
                    return false;
                if (qApp->mouseButtons() != Qt::LeftButton)
                    return false;
                QWidget *toolWindow = d->m_tabWidget->currentWidget();
                if (!toolWindow || !d->m_manager->toolWindows().contains(toolWindow))
                    return false;
                d->m_tabDragCanStart = false;
                //stop internal tab drag in QTabBar
                QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease,
                                                            static_cast<QMouseEvent*>(event)->pos(),
                                                            static_cast<QMouseEvent*>(event)->pos(),
                                                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                qApp->sendEvent(d->m_tabWidget->tabBar(), releaseEvent);
                startDrag(QWidgetList() << toolWindow);
            } else if (d->m_dragCanStart) {
                d->check_mouse_move();
            }
            break;
        default:
            break;
        }
    }
    if (event->type() == QEvent::WindowTitleChange) {
        int index = d->m_tabWidget->indexOf(static_cast<QWidget*>(object));
        if (index >= 0) {
            d->m_tabWidget->setTabText(index, d->m_tabWidget->widget(index)->windowTitle());
        }
    }
    return QAbstractToolWindowManagerArea::eventFilter(object, event);
}

void QToolWindowManagerArea::releaseTabButtons(QWidget* toolWindow) {
    Q_D(QToolWindowManagerArea);
    int index = d->m_tabWidget->indexOf(toolWindow);
    if (index < 0) {
        qWarning("unexpected indexOf fail");
        return;
    }
    if (d->m_d_manager->m_toolWindowData[toolWindow].leftButtonWidget) {
        d->m_tabWidget->tabBar()->setTabButton(
              index,
              QTabBar::LeftSide,
              0);
        d->m_d_manager->m_toolWindowData[toolWindow].leftButtonWidget->setParent(d->m_manager);
    }
    if (d->m_d_manager->m_toolWindowData[toolWindow].rightButtonWidget) {
        d->m_tabWidget->tabBar()->setTabButton(
              index,
              QTabBar::RightSide,
              0);
        d->m_d_manager->m_toolWindowData[toolWindow].rightButtonWidget->setParent(d->m_manager);
    }

}

void QToolWindowManagerArea::applyTabButtons(QWidget* toolWindow)
{
    Q_D(QToolWindowManagerArea);
    int index = d->m_tabWidget->indexOf(toolWindow);
    if (index < 0) {
        qWarning("unexpected indexOf fail");
        return;
    }
    if (d->m_d_manager->m_toolWindowData[toolWindow].leftButtonWidget) {
        d->m_tabWidget->tabBar()->setTabButton(
              index,
              QTabBar::LeftSide,
              d->m_d_manager->m_toolWindowData[toolWindow].leftButtonWidget);
    }
    if (d->m_d_manager->m_toolWindowData[toolWindow].rightButtonWidget) {
        d->m_tabWidget->tabBar()->setTabButton(
              index,
              QTabBar::RightSide,
              d->m_d_manager->m_toolWindowData[toolWindow].rightButtonWidget);
    }
}

void QToolWindowManagerArea::managerTabsClosableChanged(bool enabled)
{
    Q_D(QToolWindowManagerArea);
    d->m_tabWidget->setTabsClosable(enabled);
}

void QToolWindowManagerAreaPrivate::check_mouse_move()
{
    Q_Q(QToolWindowManagerArea);
    q->updateDragPosition();
    if (qApp->mouseButtons() == Qt::LeftButton &&
            !q->rect().contains(q->mapFromGlobal(QCursor::pos())) &&
            m_dragCanStart) {
        m_dragCanStart = false;
        q->startDrag(q->toolWindows());
    }
}

void QToolWindowManagerAreaPrivate::tabCloseRequested(int index)
{
    QWidget *toolWindow = m_tabWidget->widget(index);
    if (toolWindow && toolWindow->close()) {
        m_manager->hideToolWindow(toolWindow);
    }
}

QWidgetList QToolWindowManagerArea::toolWindows() const
{
    const Q_D(QToolWindowManagerArea);
    QWidgetList result;
    for (int i = 0; i < d->m_tabWidget->count(); i++)
        result << d->m_tabWidget->widget(i);
    return result;
}

void QToolWindowManagerArea::activateToolWindow(QWidget *toolWindow)
{
    const Q_D(QToolWindowManagerArea);
    d->m_tabWidget->setCurrentWidget(toolWindow);
}


void QToolWindowManagerArea::addToolWindows(const QWidgetList &toolWindows)
{
    Q_D(QToolWindowManagerArea);
    int index = 0;
    foreach (QWidget *toolWindow, toolWindows) {
        index = d->m_tabWidget->addTab(toolWindow, toolWindow->windowIcon(), toolWindow->windowTitle());
        applyTabButtons(toolWindow);
        toolWindow->installEventFilter(this);
    }
    d->m_tabWidget->setCurrentIndex(index);
}

void QToolWindowManagerArea::removeToolWindow(QWidget *toolWindow)
{
    Q_D(QToolWindowManagerArea);
    int index = d->m_tabWidget->indexOf(toolWindow);
    if (index < 0) {
        qWarning("QToolWindowManagerArea::removeToolWindow: no such tool window");
        return;
    }
    releaseTabButtons(toolWindow);
    d->m_tabWidget->removeTab(index);
    toolWindow->removeEventFilter(this);
}

QVariant QToolWindowManagerArea::saveState() const
{
    const Q_D(QToolWindowManagerArea);
    QVariantMap map;
    map[QLatin1String("currentIndex")] = d->m_tabWidget->currentIndex();
    return map;
}

void QToolWindowManagerArea::restoreState(const QVariant &state)
{
    Q_D(QToolWindowManagerArea);
    d->m_tabWidget->setCurrentIndex(state.toMap()[QLatin1String("currentIndex")].toInt());
}

void QToolWindowManagerArea::beforeTabButtonChanged(QWidget* toolWindow)
{
    releaseTabButtons(toolWindow);
}

void QToolWindowManagerArea::tabButtonChanged(QWidget* toolWindow)
{
    applyTabButtons(toolWindow);
}
