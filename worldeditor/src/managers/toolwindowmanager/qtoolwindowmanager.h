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

#ifndef QTOOLWINDOWMANAGER_H
#define QTOOLWINDOWMANAGER_H

#include <QtGui>

#include <QtCore/qtimer.h>
#include <QtCore/qvariant.h>

#include <QtGlobal>
#include <QWidget>
#include <QTabBar>

class QAbstractToolWindowManagerArea;
class QToolWindowManagerPrivate;
class QSplitter;
class QRubberBand;

class QToolWindowManager : public QWidget
{
    Q_OBJECT
    Q_ENUMS(AreaType ReferenceType)
    Q_PROPERTY(int suggestionSwitchInterval READ suggestionSwitchInterval
                                            WRITE setSuggestionSwitchInterval
                                            NOTIFY suggestionSwitchIntervalChanged)
    Q_PROPERTY(int borderSensitivity READ borderSensitivity
                                     WRITE setBorderSensitivity
                                     NOTIFY borderSensitivityChanged)
    Q_PROPERTY(int rubberBandLineWidth READ rubberBandLineWidth
                                       WRITE setRubberBandLineWidth
                                       NOTIFY rubberBandLineWidthChanged)
    Q_PROPERTY(bool tabsClosable READ tabsClosable
                                 WRITE setTabsClosable
                                 NOTIFY tabsClosableChanged)

public:
    explicit QToolWindowManager(QWidget *parent = 0);
    ~QToolWindowManager();

    enum AreaType {
        LastUsedArea,
        NewFloatingArea,
        EmptySpaceArea,
        NoArea
    };

    enum ReferenceType {
        ReferenceAddTo,
        ReferenceLeftOf,
        ReferenceRightOf,
        ReferenceTopOf,
        ReferenceBottomOf
    };

    void activateToolWindow(QWidget *toolWindow);

    void addToolWindow(QWidget *toolWindow, AreaType area = LastUsedArea);
    void addToolWindow(QWidget *toolWindow, ReferenceType reference,
                       QAbstractToolWindowManagerArea *area);
    void addToolWindows(const QWidgetList &toolWindows, AreaType area = LastUsedArea);
    void addToolWindows(const QWidgetList &toolWindows, ReferenceType reference,
                        QAbstractToolWindowManagerArea *area);

    void moveToolWindow(QWidget *toolWindow, AreaType area = LastUsedArea);
    void moveToolWindow(QWidget *toolWindow, ReferenceType reference,
                        QAbstractToolWindowManagerArea *area);
    void moveToolWindows(const QWidgetList &toolWindows, AreaType area = LastUsedArea);
    void moveToolWindows(const QWidgetList &toolWindows, ReferenceType reference,
                         QAbstractToolWindowManagerArea *area);

    QAbstractToolWindowManagerArea *areaFor(QWidget *toolWindow) const;
    void removeToolWindow(QWidget *toolWindow);
    QWidgetList toolWindows() const;
    void hideToolWindow(QWidget *toolWindow);
    QVariant saveState() const;
    void restoreState(const QVariant& data);

    int suggestionSwitchInterval() const;
    void setSuggestionSwitchInterval(int msec);

    int borderSensitivity() const;
    void setBorderSensitivity(int pixels);

    int rubberBandLineWidth() const;
    void setRubberBandLineWidth(int pixels);

    bool tabsClosable() const;
    void setTabsClosable(bool enabled);

    QRubberBand *rectRubberBand() const;
    QRubberBand *lineRubberBand() const;

    void setTabButton(QWidget *toolWindow, QTabBar::ButtonPosition position, QWidget *widget);

signals:
    void toolWindowVisibilityChanged(QWidget *toolWindow, bool visible);
    void suggestionSwitchIntervalChanged(int suggestionSwitchInterval);
    void borderSensitivityChanged(int borderSensitivity);
    void rubberBandLineWidthChanged(int rubberBandLineWidth);
    void tabsClosableChanged(bool tabsClosable);

protected:
    virtual QSplitter *createSplitter();
    virtual QAbstractToolWindowManagerArea *createArea();
    virtual QPixmap generateDragPixmap(const QWidgetList &toolWindows);
    bool event(QEvent *e);

private:
    Q_DISABLE_COPY(QToolWindowManager)
    Q_DECLARE_PRIVATE(QToolWindowManager)
    QToolWindowManagerPrivate *d_ptr;

    friend class QToolWindowManagerWrapper;
    friend class QAbstractToolWindowManagerArea;
    friend class QToolWindowManagerArea;
};

#endif // QTOOLWINDOWMANAGER_H
