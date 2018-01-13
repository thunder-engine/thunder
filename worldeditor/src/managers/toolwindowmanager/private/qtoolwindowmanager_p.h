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

#ifndef QTOOLWINDOWMANAGER_P_H
#define QTOOLWINDOWMANAGER_P_H

#include <QtGlobal>
#include <QWidget>
#include "../qtoolwindowmanager.h"
#include "qtoolwindowmanagerarea_p.h"

class QToolWindowManagerAreaReference
{
public:
    QToolWindowManagerAreaReference(QToolWindowManager::AreaType type = QToolWindowManager::NoArea);
    QToolWindowManagerAreaReference(QToolWindowManager::ReferenceType type, QWidget *widget);
    QToolWindowManager::AreaType areaType() const { return m_areaType; }
    QToolWindowManager::ReferenceType referenceType() const { return m_referenceType; }
    bool isReference() const { return m_widget != 0; }
    QWidget *widget() const { return m_widget; }

private:
    QToolWindowManager::AreaType m_areaType;
    QToolWindowManager::ReferenceType m_referenceType;
    QWidget *m_widget;
};

class QSplitter;
class QTabWidget;
class QTabBar;
class QLabel;
class QToolWindowManagerWrapper;
class QToolWindowManagerPrivate;

class QToolWindowManagerPrivateSlots : public QObject {
    Q_OBJECT
public:
    QToolWindowManagerPrivate* d;
public Q_SLOTS:
    void showNextDropSuggestion();
    //void tabCloseRequested(int index);
    void areaDestroyed(QObject *object);
};


class QToolWindowManagerPrivate
{
    Q_DECLARE_PUBLIC(QToolWindowManager)
public:
    void addToolWindows(QList<QWidget*> toolWindows, const QToolWindowManagerAreaReference& area);
    void moveToolWindows(const QWidgetList &toolWindows, const QToolWindowManagerAreaReference& area);

    // all added tool windows
    QList<QWidget *> m_toolWindows;
    // all areas for this manager
    QList<QAbstractToolWindowManagerArea *> m_areas;
    // all wrappers for this manager
    QList<QToolWindowManagerWrapper *> m_wrappers;
    int m_borderSensitivity;
    int m_rubberBandLineWidth;
    bool m_tabsClosable;
    // list of tool windows that are currently dragged, or empty list if there is no current drag
    QList<QWidget *> m_draggedToolWindows;
    // label used to display dragged content
    QLabel *m_dragIndicator;

    // placeholder objects used for displaying drop suggestions
    QRubberBand *m_rectRubberBand;
    QRubberBand *m_lineRubberBand;
    // full list of suggestions for current cursor position
    QList<QToolWindowManagerAreaReference> m_suggestions;
    // index of currently displayed drop suggestion
    // (e.g. always 0 if there is only one possible drop location)
    int m_dropCurrentSuggestionIndex;
    // used for switching drop suggestions
    QTimer m_dropSuggestionSwitchTimer;
    // last widget used for adding tool windows, or 0 if there isn't one
    // (warning: may contain pointer to deleted object)
    QAbstractToolWindowManagerArea *m_lastUsedArea;
    void handleNoSuggestions();
    // remove tool window from its area (if any) and set parent to 0
    void releaseToolWindow(QWidget *toolWindow);
    // remove constructions that became useless
    void simplifyLayout();
    void startDrag(const QWidgetList &toolWindows);

    QVariantMap saveAreaState(QAbstractToolWindowManagerArea *area);
    QAbstractToolWindowManagerArea *restoreAreaState(const QVariantMap& data);

    QVariantMap saveSplitterState(QSplitter *splitter);
    QSplitter *restoreSplitterState(const QVariantMap& data);
    void findSuggestions(QToolWindowManagerWrapper *wrapper);
    QRect sideSensitiveArea(QWidget *widget, QToolWindowManager::ReferenceType side);
    QRect sidePlaceHolderRect(QWidget *widget, QToolWindowManager::ReferenceType side);

    QSplitter *createAndSetupSplitter();

    void updateDragPosition();
    void finishDrag();
    bool dragInProgress() { return !m_draggedToolWindows.isEmpty(); }

    QToolWindowManagerPrivateSlots slots_object;
    void showNextDropSuggestion();
    //void tabCloseRequested(int index, QObject *sender);

    QAbstractToolWindowManagerArea * createAndSetupArea();

    struct ToolWindowData {
      ToolWindowData() : leftButtonWidget(0), rightButtonWidget(0) {}
      QWidget *leftButtonWidget;
      QWidget *rightButtonWidget;
    };

    QHash<QWidget *, ToolWindowData> m_toolWindowData;

    QToolWindowManager *q_ptr;

};

class QToolWindowManagerAreaPrivateSlots : public QObject {
    Q_OBJECT
public:
    QToolWindowManagerAreaPrivate *d;
public Q_SLOTS:
    void tabCloseRequested(int index);
};

class QToolWindowManagerAreaPrivate
{
    Q_DECLARE_PUBLIC(QToolWindowManagerArea)
public:
    QToolWindowManager *m_manager;
    QToolWindowManagerPrivate *m_d_manager;

    QTabWidget *m_tabWidget;

    // indicates that user has started mouse movement on QTabWidget
    // that can be considered as dragging it if the cursor will leave
    // its area
    bool m_dragCanStart;

    // indicates that user has started mouse movement on QTabWidget
    // that can be considered as dragging current tab
    // if the cursor will leave the tab bar area
    bool m_tabDragCanStart;

    QToolWindowManagerAreaPrivateSlots slots_object;

    void addToolWindow(QWidget *toolWindow);
    void addToolWindows(const QList<QWidget*>& toolWindows);

    // check if mouse left tab widget area so that dragging should start
    void check_mouse_move();

    void tabCloseRequested(int index);

    QToolWindowManagerArea* q_ptr;
};

#endif // QTOOLWINDOWMANAGER_P_H
