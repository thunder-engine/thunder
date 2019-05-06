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

#include "qtoolwindowmanager.h"

#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qrubberband.h>
#include "qabstracttoolwindowmanagerarea.h"
#include "private/qtoolwindowmanagerarea_p.h"
#include "private/qtoolwindowmanagerwrapper_p.h"
#include "private/qtoolwindowmanager_p.h"

template<class T>
T findClosestParent(QWidget *widget)
{
    while (widget) {
        if (T result = qobject_cast<T>(widget))
            return result;
        widget = widget->parentWidget();
    }
    return nullptr;
}

/*!
    \class QToolWindowManager

    \brief The QToolWindowManager class provides docking tool behavior.

    \inmodule QtWidgets

    \since 5.4

    The behavior provided by QToolWindowManager is similar to tool windows mechanism
    in Visual Studio or Eclipse.
    User can arrange tool windows in tabs, dock it to any border, split with vertical
    and horizontal splitters, tabify them together and detach to floating windows.
    QToolWindowManager can be used to implement ability for user to setup an
    environment with customized tool windows layout.

    QToolWindowManager is in many ways similar to QMainWindow that also provides ability to
    pin dock widgets and toolbars to its borders, tabify or detach them. However,
    QToolWindowManager has extended abilities to manage tool windows.
    It has no canonic central area. Each and every area can be used
    to tabify several tool windows. Each area can be divided to display many
    tool windows at once. Dropping tool windows is not limited to the borders
    of QToolWindowManager. A tool window can be dropped at any border between areas.
    These features also applies to the floating areas. They can be split up and/or
    contain tabified tool windows.

    QToolWindowManager also provides API to setup an arbitrary environment programmatically.
    This can be used to create presets of environments for quick start.

    Each tool window can be dragged individually by pressing left mouse button at
    its tab button at the tab bar and dragging away. Each area containing multiple tabified
    tool window can be dragged as a whole by pressing left mouse button at the empty space
    at the tab bar and dragging away. When an item is dragged and mouse is over a suitable
    drop area (i.e. QToolWindowManager itself or one of the floating windows created by it),
    a drop suggestion appears indicating where the item will be put to if it's
    dropped. It can be either a line indicating that the item will be inserted between its
    neighbor areas, or a rectangle indicating that the dragged tool window(s) will be added
    to the highlighted area as tabs. When an item is dropped to space that is not managed by
    the QToolWindowManager, it becomes a floating window.

*/
/*!
    \property QToolWindowManager::suggestionSwitchInterval
    \brief The delay between showing the next suggestion of drop location in milliseconds.

    When user starts a tool window drag and moves mouse pointer to a position, there can be
    an ambiguity in new position of the tool window. If user holds the left mouse button and
    stops mouse movements, all possible suggestions will be indicated periodically, one at a time.

    Default value is 1000 (i.e. 1 second).
*/
/*!
    \property QToolWindowManager::borderSensitivity
    \brief Maximal distance in pixels between mouse position and area border that allows
    to display a suggestion.

    Default value is 12.
*/
/*!
    \property QToolWindowManager::rubberBandLineWidth
    \brief Visible width of rubber band line that is used to display drop suggestions.

    Default value is the same as QSplitter::handleWidth default value on current platform.
*/
/*!
    \enum QToolWindowManager::AreaType

    Describes where to place a tool window.

    \value LastUsedArea The area tool windows has been added to most recently.
    \value NewFloatingArea New area in a detached window.
    \value EmptySpaceArea Area inside the manager widget (only available when there is
    no tool windows in it).
    \value NoArea Tool window is hidden.
*/
/*!
    \enum QToolWindowManager::ReferenceType

    Describes where to place a tool window relative to an existing area.

    \value ReferenceAddTo Add to existing QToolWindowManagerArea as new tab.
    \value ReferenceLeftOf Add to new area to the left of the QToolWindowManagerArea.
    \value ReferenceRightOf Add to new area to the right of the QToolWindowManagerArea.
    \value ReferenceTopOf Add to new area to the top of the QToolWindowManagerArea.
    \value ReferenceBottomOf Add to new area to the bottom of the QToolWindowManagerArea.

*/
/*!
    \fn void QToolWindowManager::toolWindowVisibilityChanged(QWidget* toolWindow, bool visible)

    This signal is emitted when \a toolWindow may be hidden or shown.
    \a visible indicates new visibility state of the tool window.
*/

/*!
 * \brief Creates a manager with given \a parent.
 */
QToolWindowManager::QToolWindowManager(QWidget *parent) :
    QWidget(parent, nullptr)
{
    d_ptr = new QToolWindowManagerPrivate();
    d_ptr->q_ptr = this;
    Q_D(QToolWindowManager);
    d->slots_object.d = d;
    d->m_lastUsedArea = nullptr;
    d->m_borderSensitivity = 12;
    d->m_tabsClosable = true;
    QSplitter *testSplitter = new QSplitter();
    d->m_rubberBandLineWidth = testSplitter->handleWidth();
    delete testSplitter;
    d->m_dragIndicator = new QLabel(nullptr, Qt::ToolTip );
    d->m_dragIndicator->setAttribute(Qt::WA_ShowWithoutActivating);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QToolWindowManagerWrapper* wrapper = new QToolWindowManagerWrapper(this);
    wrapper->setWindowFlags(wrapper->windowFlags() & ~Qt::Tool);
    mainLayout->addWidget(wrapper);
    connect(&(d->m_dropSuggestionSwitchTimer), SIGNAL(timeout()),
            &(d->slots_object), SLOT(showNextDropSuggestion()));
    d->m_dropSuggestionSwitchTimer.setInterval(1000);
    d->m_dropCurrentSuggestionIndex = 0;

    d->m_rectRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    d->m_lineRubberBand = new QRubberBand(QRubberBand::Line, this);
}

/*!
 * \brief Destroys the widget. Additionally all tool windows and all floating windows
 * created by this widget are destroyed.
 */
QToolWindowManager::~QToolWindowManager()
{
    Q_D(QToolWindowManager);
    qDeleteAll(d->m_areas);
    d->m_areas.clear();
    qDeleteAll(d->m_wrappers);
    d->m_wrappers.clear();
    delete d_ptr;
}

void QToolWindowManager::activateToolWindow(QWidget *toolWindow)
{
    Q_D(QToolWindowManager);
    foreach(QAbstractToolWindowManagerArea *it, d->m_areas) {
        it->activateToolWindow(toolWindow);
    }
}

/*!
 * Adds \a toolWindow to the manager and moves it to the position specified in
 * \a area. This function is a shortcut for QToolWindowManager::addToolWindows.
 */
void QToolWindowManager::addToolWindow(QWidget *toolWindow,
                                       QToolWindowManager::AreaType area)
{
    Q_D(QToolWindowManager);
    d->addToolWindows(QWidgetList() << toolWindow,
                      QToolWindowManagerAreaReference(area));
}

/*!
  \overload

  Adds \a toolWindow to the manager and moves it to the position specified by
  a \a reference relative to \a area.
  This function is a shortcut for QToolWindowManager::addToolWindows.
 */
void QToolWindowManager::addToolWindow(QWidget *toolWindow,
                                       QToolWindowManager::ReferenceType reference,
                                       QAbstractToolWindowManagerArea *area)
{
    Q_D(QToolWindowManager);
    d->addToolWindows(QWidgetList() << toolWindow,
                      QToolWindowManagerAreaReference(reference, area));
}

/*!
 * \brief Adds \a toolWindows to the manager and moves it to the position specified in \a area.
 * The manager takes ownership of the tool windows and will delete them upon destruction.
 *
 * toolWindow->windowIcon() and toolWindow->windowTitle() will be used as the icon and title
 * of the tab that represents the tool window.
 *
 * If you intend to use QToolWindowManager::saveState
 * and QToolWindowManager::restoreState functions, you must set objectName() of each added
 * tool window to a non-empty unique string.
 */
void QToolWindowManager::addToolWindows(const QWidgetList &toolWindows,
                                        QToolWindowManager::AreaType area)
{
    Q_D(QToolWindowManager);
    d->addToolWindows(toolWindows,
                      QToolWindowManagerAreaReference(area));
}

/*!
  \overload

  Adds \a toolWindows to the manager and moves it to the position specified by
  a \a reference relative to \a area.
*/
void QToolWindowManager::addToolWindows(const QWidgetList &toolWindows,
                                        QToolWindowManager::ReferenceType reference,
                                        QAbstractToolWindowManagerArea *area)
{
    Q_D(QToolWindowManager);
    d->addToolWindows(toolWindows,
                      QToolWindowManagerAreaReference(reference, area));
}

/*!
 * \brief Moves \a toolWindow to the position specified in \a area.
 *
 * \a toolWindow must be added to the manager prior to calling this function.
 */
void QToolWindowManager::moveToolWindow(QWidget *toolWindow,
                                       QToolWindowManager::AreaType area)
{
    Q_D(QToolWindowManager);
    d->moveToolWindows(QWidgetList() << toolWindow,
                      QToolWindowManagerAreaReference(area));
}

/*!
  \overload

  \brief Moves \a toolWindow to the position specified by a \a reference relative to \a area.
*/
void QToolWindowManager::moveToolWindow(QWidget *toolWindow,
                                       QToolWindowManager::ReferenceType reference,
                                       QAbstractToolWindowManagerArea *area)
{
    Q_D(QToolWindowManager);
    d->moveToolWindows(QWidgetList() << toolWindow,
                      QToolWindowManagerAreaReference(reference, area));
}
/*!
 * \brief Moves \a toolWindows to the position specified in \a area.
 *
 * \a toolWindows must be added to the manager prior to calling this function.
 */
void QToolWindowManager::moveToolWindows(const QWidgetList &toolWindows,
                                        QToolWindowManager::AreaType area)
{
    Q_D(QToolWindowManager);
    d->moveToolWindows(toolWindows,
                      QToolWindowManagerAreaReference(area));
}

/*!
  \overload

  \brief Moves \a toolWindows to the position specified by a \a reference relative to \a area.
*/
void QToolWindowManager::moveToolWindows(const QWidgetList &toolWindows,
                                        QToolWindowManager::ReferenceType reference,
                                        QAbstractToolWindowManagerArea *area)
{
    Q_D(QToolWindowManager);
    d->moveToolWindows(toolWindows,
                      QToolWindowManagerAreaReference(reference, area));
}

void QToolWindowManagerPrivate::addToolWindows(QList<QWidget *> toolWindows,
                                        const QToolWindowManagerAreaReference &area)
{
    foreach (QWidget* toolWindow, toolWindows) {
        if (!toolWindow) {
            qWarning("cannot add null widget");
            continue;
        }
        if (m_toolWindows.contains(toolWindow)) {
            qWarning("this tool window has already been added");
            continue;
        }
        toolWindow->hide();
        toolWindow->setParent(nullptr);
        m_toolWindows << toolWindow;
    }
    moveToolWindows(toolWindows, area);
}

/*!
 * Returns the area that contains \a toolWindow, or 0 if \a toolWindow is hidden.
 */
QAbstractToolWindowManagerArea *QToolWindowManager::areaFor(QWidget *toolWindow) const
{
    return findClosestParent<QAbstractToolWindowManagerArea *>(toolWindow);
}

void QToolWindowManagerPrivate::moveToolWindows(const QWidgetList &toolWindows,
                                         const QToolWindowManagerAreaReference& area_param)
{
    Q_Q(QToolWindowManager);
    QToolWindowManagerAreaReference area = area_param;
    foreach (QWidget *toolWindow, toolWindows) {
        if (!m_toolWindows.contains(toolWindow)) {
            qWarning("unknown tool window");
            return;
        }
        if (toolWindow->parentWidget() != nullptr)
            releaseToolWindow(toolWindow);
    }
    if (!area.isReference() && area.areaType() == QToolWindowManager::LastUsedArea && !m_lastUsedArea) {
        QAbstractToolWindowManagerArea *foundArea = q->findChild<QAbstractToolWindowManagerArea *>();
        if (foundArea)
            area = QToolWindowManagerAreaReference(QToolWindowManager::ReferenceAddTo, foundArea);
        else
            area = QToolWindowManager::EmptySpaceArea;
    }

    if (!area.isReference() && area.areaType() == QToolWindowManager::NoArea) {
        //do nothing
    } else if (!area.isReference() && area.areaType() == QToolWindowManager::NewFloatingArea) {
        QAbstractToolWindowManagerArea *area = createAndSetupArea();
        area->addToolWindows(toolWindows);
        m_lastUsedArea = area;
        QToolWindowManagerWrapper *wrapper = new QToolWindowManagerWrapper(q);
        wrapper->layout()->addWidget(area);
        wrapper->move(QCursor::pos());
        wrapper->show();
    } else if (area.isReference() && area.referenceType() == QToolWindowManager::ReferenceAddTo) {
        QAbstractToolWindowManagerArea *area2 =
                static_cast<QAbstractToolWindowManagerArea*>(area.widget());
        area2->addToolWindows(toolWindows);
        m_lastUsedArea = area2;
    } else if (area.isReference()) {
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(area.widget()->parentWidget());
        QToolWindowManagerWrapper *wrapper =
                qobject_cast<QToolWindowManagerWrapper*>(area.widget()->parentWidget());
        if (!parentSplitter && !wrapper) {
            qWarning("unknown parent type");
            return;
        }
        bool useParentSplitter = false;
        int indexInParentSplitter = 0;
        if (parentSplitter) {
            indexInParentSplitter = parentSplitter->indexOf(area.widget());
            if (parentSplitter->orientation() == Qt::Vertical)
                useParentSplitter = area.referenceType() == QToolWindowManager::ReferenceTopOf ||
                                    area.referenceType() == QToolWindowManager::ReferenceBottomOf;
            else
                useParentSplitter = area.referenceType() == QToolWindowManager::ReferenceLeftOf ||
                                    area.referenceType() == QToolWindowManager::ReferenceRightOf;
        }
        if (useParentSplitter) {
            if (area.referenceType() == QToolWindowManager::ReferenceBottomOf ||
                area.referenceType() == QToolWindowManager::ReferenceRightOf)
                indexInParentSplitter++;
            QAbstractToolWindowManagerArea *newArea = createAndSetupArea();
            newArea->addToolWindows(toolWindows);
            m_lastUsedArea = newArea;
            parentSplitter->insertWidget(indexInParentSplitter, newArea);
        } else {
            area.widget()->hide();
            area.widget()->setParent(nullptr);
            QSplitter *splitter = createAndSetupSplitter();
            if (area.referenceType() == QToolWindowManager::ReferenceTopOf ||
                area.referenceType() == QToolWindowManager::ReferenceBottomOf)
                splitter->setOrientation(Qt::Vertical);
            else
                splitter->setOrientation(Qt::Horizontal);
            splitter->addWidget(area.widget());
            area.widget()->show();
            QAbstractToolWindowManagerArea *newArea = createAndSetupArea();
            if (area.referenceType() == QToolWindowManager::ReferenceTopOf ||
                area.referenceType() == QToolWindowManager::ReferenceLeftOf)
                splitter->insertWidget(0, newArea);
            else
                splitter->addWidget(newArea);
            if (parentSplitter)
                parentSplitter->insertWidget(indexInParentSplitter, splitter);
            else
                wrapper->layout()->addWidget(splitter);
            newArea->addToolWindows(toolWindows);
            m_lastUsedArea = newArea;
        }
    } else if (!area.isReference() && area.areaType() == QToolWindowManager::EmptySpaceArea) {
        QAbstractToolWindowManagerArea *newArea = createAndSetupArea();
        q->findChild<QToolWindowManagerWrapper*>()->layout()->addWidget(newArea);
        newArea->addToolWindows(toolWindows);
        m_lastUsedArea = newArea;
    } else if (!area.isReference() && area.areaType() == QToolWindowManager::LastUsedArea) {
        m_lastUsedArea->addToolWindows(toolWindows);
    } else {
        qWarning("invalid type");
    }
    simplifyLayout();
    foreach (QWidget *toolWindow, toolWindows)
        emit q->toolWindowVisibilityChanged(toolWindow, toolWindow->parent() != nullptr);
}

/*!
 * \brief Removes \a toolWindow from the manager. \a toolWindow becomes a hidden
 * top level widget. The ownership of \a toolWindow is returned to the caller.
 */
void QToolWindowManager::removeToolWindow(QWidget *toolWindow)
{
    Q_D(QToolWindowManager);
    if (!d->m_toolWindows.contains(toolWindow)) {
        qWarning("unknown tool window");
        return;
    }
    moveToolWindow(toolWindow, NoArea);
    d->m_toolWindows.removeOne(toolWindow);
}

/*!
 * \brief Returns all tool window added to the manager.
 */
QWidgetList QToolWindowManager::toolWindows() const
{
    const Q_D(QToolWindowManager);
    return d->m_toolWindows;
}

/*!
 * Hides \a toolWindow.
 *
 * \a toolWindow must be added to the manager prior to calling this function.
 */
void QToolWindowManager::hideToolWindow(QWidget *toolWindow)
{
    QWidget *p  = toolWindow->parentWidget();
    if(p) {
        moveToolWindow(toolWindow, NoArea);
    }
}

void QToolWindowManager::setSuggestionSwitchInterval(int msec)
{
    Q_D(QToolWindowManager);
    d->m_dropSuggestionSwitchTimer.setInterval(msec);
    emit suggestionSwitchIntervalChanged(msec);
}

int QToolWindowManager::suggestionSwitchInterval() const
{
    const Q_D(QToolWindowManager);
    return d->m_dropSuggestionSwitchTimer.interval();
}

int QToolWindowManager::borderSensitivity() const
{
    const Q_D(QToolWindowManager);
    return d->m_borderSensitivity;
}

void QToolWindowManager::setBorderSensitivity(int pixels)
{
    Q_D(QToolWindowManager);
    d->m_borderSensitivity = pixels;
    emit borderSensitivityChanged(pixels);
}

void QToolWindowManager::setRubberBandLineWidth(int pixels)
{
    Q_D(QToolWindowManager);
    d->m_rubberBandLineWidth = pixels;
    emit rubberBandLineWidthChanged(pixels);
}

bool QToolWindowManager::tabsClosable() const
{
    const Q_D(QToolWindowManager);
    return d->m_tabsClosable;
}

void QToolWindowManager::setTabsClosable(bool enabled)
{
    Q_D(QToolWindowManager);
    if (d->m_tabsClosable != enabled) {
        d->m_tabsClosable = enabled;
        emit tabsClosableChanged(enabled);
    }
}

int QToolWindowManager::rubberBandLineWidth() const
{
    const Q_D(QToolWindowManager);
    return d->m_rubberBandLineWidth;
}

/*!
 * Returns the widget that is used to display rectangular drop suggestions.
 */
QRubberBand *QToolWindowManager::rectRubberBand() const
{
    const Q_D(QToolWindowManager);
    return d->m_rectRubberBand;
}

/*!
 * Returns the widget that is used to display line drop suggestions.
 */
QRubberBand *QToolWindowManager::lineRubberBand() const
{
    const Q_D(QToolWindowManager);
    return d->m_lineRubberBand;
}

void QToolWindowManager::setTabButton(QWidget* toolWindow, QTabBar::ButtonPosition position, QWidget* widget) {
  Q_D(QToolWindowManager);
  QAbstractToolWindowManagerArea *area = areaFor(toolWindow);
  if (area)
      area->beforeTabButtonChanged(toolWindow);
  switch(position) {
  case QTabBar::LeftSide:
      d->m_toolWindowData[toolWindow].leftButtonWidget = widget;
      break;
  case QTabBar::RightSide:
      d->m_toolWindowData[toolWindow].rightButtonWidget = widget;
      break;
  }
  if (area)
      area->tabButtonChanged(toolWindow);
}

/*!
 * Create a splitter. Reimplement this function if you want to use your own splitter subclass.
 */
QSplitter *QToolWindowManager::createSplitter()
{
    QSplitter *splitter = new QSplitter();
    splitter->setChildrenCollapsible(false);
    return splitter;
}

/*!
 * Dumps the state and position of all tool windows to a variable. It can be stored in application settings
 * using QSettings. Stored state can be restored by calling QToolWindowManager::restoreState
 * with the same value.
 */
QVariant QToolWindowManager::saveState() const
{
    const Q_D(QToolWindowManager);
    QVariantMap result;
    result[QLatin1String("QToolWindowManagerStateFormat")] = 1;
    QToolWindowManagerWrapper *mainWrapper = findChild<QToolWindowManagerWrapper*>();
    if (!mainWrapper) {
        qWarning("can't find main wrapper");
        return QVariant();
    }
    result[QLatin1String("mainWrapper")] = mainWrapper->saveState();
    QVariantList floatingWindowsData;
    foreach (QToolWindowManagerWrapper *wrapper, d->m_wrappers) {
        if (!wrapper->isWindow())
            continue;
        floatingWindowsData << wrapper->saveState();
    }
    result[QLatin1String("floatingWindows")] = floatingWindowsData;
    return result;
}

/*!
  Restores state and position of tool windows stored in \a data.
*/
void QToolWindowManager::restoreState(const QVariant &data)
{
    Q_D(QToolWindowManager);
    if (!data.isValid())
        return;
    QVariantMap dataMap = data.toMap();
    if (dataMap[QLatin1String("QToolWindowManagerStateFormat")].toInt() != 1) {
        qWarning("state format is not recognized");
        return;
    }
    moveToolWindows(d->m_toolWindows, NoArea);
    QToolWindowManagerWrapper *mainWrapper = findChild<QToolWindowManagerWrapper*>();
    if (!mainWrapper) {
        qWarning("can't find main wrapper");
        return;
    }
    mainWrapper->restoreState(dataMap[QLatin1String("mainWrapper")].toMap());
    foreach (QVariant windowData, dataMap[QLatin1String("floatingWindows")].toList()) {
        QToolWindowManagerWrapper *wrapper = new QToolWindowManagerWrapper(this);
        wrapper->restoreState(windowData.toMap());
        wrapper->show();
    }
    d->simplifyLayout();
    foreach (QWidget *toolWindow, d->m_toolWindows)
        emit toolWindowVisibilityChanged(toolWindow, toolWindow->parentWidget() != nullptr);
}


void QToolWindowManagerPrivate::handleNoSuggestions()
{
    Q_Q(QToolWindowManager);
    m_rectRubberBand->hide();
    m_lineRubberBand->hide();
    m_lineRubberBand->setParent(q);
    m_rectRubberBand->setParent(q);
    m_suggestions.clear();
    m_dropCurrentSuggestionIndex = 0;
    if (m_dropSuggestionSwitchTimer.isActive())
        m_dropSuggestionSwitchTimer.stop();
}

void QToolWindowManagerPrivate::releaseToolWindow(QWidget *toolWindow)
{
    QAbstractToolWindowManagerArea *previousArea =
            findClosestParent<QAbstractToolWindowManagerArea *>(toolWindow);
    if (!previousArea) {
        qWarning("cannot find tab widget for tool window");
        return;
    }
    previousArea->removeToolWindow(toolWindow);
    toolWindow->hide();
    toolWindow->setParent(nullptr);
}

void QToolWindowManagerPrivate::simplifyLayout()
{
    foreach (QAbstractToolWindowManagerArea *area, m_areas) {
        if (area->parentWidget() == nullptr) {
            if (area->toolWindows().isEmpty()) {
                area->deleteLater();
            }
            continue;
        }
        QSplitter *splitter = qobject_cast<QSplitter*>(area->parentWidget());
        QSplitter *validSplitter = nullptr; // least top level splitter that should remain
        QSplitter *invalidSplitter = nullptr; //most top level splitter that should be deleted
        while (splitter) {
            if (splitter->count() > 1) {
                validSplitter = splitter;
                break;
            }
            invalidSplitter = splitter;
            splitter = qobject_cast<QSplitter*>(splitter->parentWidget());
        }
        if (!validSplitter) {
            QToolWindowManagerWrapper *wrapper =
                    findClosestParent<QToolWindowManagerWrapper*>(area);
            if (!wrapper) {
                qWarning("can't find wrapper");
                return;
            }
            if (area->toolWindows().isEmpty() && wrapper->isWindow()) {
                wrapper->hide();
                wrapper->deleteLater();
            } else if (area->parent() != wrapper) {
                wrapper->layout()->addWidget(area);
            }
        } else if (!area->toolWindows().isEmpty() &&
                   validSplitter &&
                   area->parent() != validSplitter) {
            int index = validSplitter->indexOf(invalidSplitter);
            validSplitter->insertWidget(index, area);
        }
        if (invalidSplitter) {
            invalidSplitter->hide();
            invalidSplitter->setParent(nullptr);
            invalidSplitter->deleteLater();
        }
        if (area->toolWindows().isEmpty()) {
            area->hide();
            area->setParent(nullptr);
            area->deleteLater();
        }
    }
}

void QToolWindowManagerPrivate::startDrag(const QWidgetList &toolWindows)
{
    Q_Q(QToolWindowManager);
    if (dragInProgress()) {
        qWarning("QToolWindowManager::execDrag: drag is already in progress");
        return;
    }
    if (toolWindows.isEmpty())
        return;
    m_draggedToolWindows = toolWindows;
    m_dragIndicator->setPixmap(q->generateDragPixmap(toolWindows));
    updateDragPosition();
    m_dragIndicator->show();
}

QVariantMap QToolWindowManagerPrivate::saveAreaState(QAbstractToolWindowManagerArea *area) {
    QVariantMap result;
    result[QLatin1String("type")] = QLatin1String("area");
    QStringList objectNames;
    foreach (QWidget *widget, area->toolWindows()) {
        QString name = widget->objectName();
        if (name.isEmpty())
            qWarning("cannot save state of tool window without object name");
        else
            objectNames << name;
    }
    result[QLatin1String("objectNames")] = objectNames;
    result[QLatin1String("customData")] = area->saveState();
    return result;
}

QAbstractToolWindowManagerArea *QToolWindowManagerPrivate::restoreAreaState(const QVariantMap &data) {
    QWidgetList toolWindows;
    foreach (QVariant objectNameValue, data[QLatin1String("objectNames")].toList()) {
        QString objectName = objectNameValue.toString();
        if (objectName.isEmpty()) { continue; }
        bool found = false;
        foreach (QWidget *toolWindow, m_toolWindows) {
            if (toolWindow->objectName() == objectName) {
                toolWindows << toolWindow;
                found = true;
                break;
            }
        }
        if (!found)
            qWarning("tool window with name '%s' not found", objectName.toLocal8Bit().constData());
    }
    QAbstractToolWindowManagerArea *area = createAndSetupArea();
    area->addToolWindows(toolWindows);
    area->restoreState(data[QLatin1String("customData")]);
    return area;
}

QVariantMap QToolWindowManagerPrivate::saveSplitterState(QSplitter *splitter)
{
    QVariantMap result;
    result[QLatin1String("state")] = splitter->saveState();
    result[QLatin1String("type")] = QLatin1String("splitter");
    QVariantList items;
    for (int i = 0; i < splitter->count(); i++) {
        QWidget *item = splitter->widget(i);
        QVariantMap itemValue;
        QAbstractToolWindowManagerArea *area = qobject_cast<QAbstractToolWindowManagerArea *>(item);
        if (area) {
            itemValue = saveAreaState(area);
        } else {
            QSplitter *childSplitter = qobject_cast<QSplitter*>(item);
            if (childSplitter)
                itemValue = saveSplitterState(childSplitter);
            else
                qWarning("unknown splitter item");
        }
        items << itemValue;
    }
    result[QLatin1String("items")] = items;
    return result;
}

QSplitter *QToolWindowManagerPrivate::restoreSplitterState(const QVariantMap &data)
{
    //Q_Q(QToolWindowManager);
    if (data[QLatin1String("items")].toList().count() < 2)
        qWarning("invalid splitter encountered");

    QSplitter *splitter = createAndSetupSplitter();
    foreach (QVariant itemData, data[QLatin1String("items")].toList()) {
        QVariantMap itemValue = itemData.toMap();
        QString itemType = itemValue[QLatin1String("type")].toString();
        if (itemType == QLatin1String("splitter")) {
            splitter->addWidget(restoreSplitterState(itemValue));
        } else if (itemType == QLatin1String("area")) {
            splitter->addWidget(restoreAreaState(itemValue));
        } else {
            qWarning("unknown item type");
        }
    }
    splitter->restoreState(data[QLatin1String("state")].toByteArray());
    return splitter;
}

/*!
 * Generates a pixmap for \a toolWindows that is used to represent the data
 * in a drag and drop operation near the mouse cursor.
 * You may reimplement this function to use different pixmaps.
 */
QPixmap QToolWindowManager::generateDragPixmap(const QList<QWidget *> &toolWindows)
{
    QTabBar widget;
    //widget.setDocumentMode(true);
    foreach (QWidget *toolWindow, toolWindows) {
        widget.addTab(toolWindow->windowIcon(), toolWindow->windowTitle());
    }
    #if QT_VERSION >= 0x050000 // Qt5
        return widget.grab();
    #else //Qt4
        return QPixmap::grabWidget(&widget);
    #endif
}

void QToolWindowManagerPrivate::showNextDropSuggestion()
{
    Q_Q(QToolWindowManager);
    if (m_suggestions.isEmpty()) {
        qWarning("showNextDropSuggestion called but no suggestions");
        return;
    }
    m_dropCurrentSuggestionIndex++;
    if (m_dropCurrentSuggestionIndex >= m_suggestions.count())
        m_dropCurrentSuggestionIndex = 0;
    const QToolWindowManagerAreaReference& suggestion =
            m_suggestions[m_dropCurrentSuggestionIndex];
    if ( (suggestion.isReference() && suggestion.referenceType() == QToolWindowManager::ReferenceAddTo) ||
         (!suggestion.isReference() && suggestion.areaType() == QToolWindowManager::EmptySpaceArea) ) {
        QWidget *widget;
        if (!suggestion.isReference() &&
                suggestion.areaType() == QToolWindowManager::EmptySpaceArea)
            widget = q->findChild<QToolWindowManagerWrapper*>();
        else
            widget = suggestion.widget();
        QWidget *placeHolderParent;
        if (widget->topLevelWidget() == q->topLevelWidget())
            placeHolderParent = q;
        else
            placeHolderParent = widget->topLevelWidget();
        QRect placeHolderGeometry = widget->rect();
        placeHolderGeometry.moveTopLeft(widget->mapTo(placeHolderParent,
                                                      placeHolderGeometry.topLeft()));
        m_rectRubberBand->setGeometry(placeHolderGeometry);
        m_rectRubberBand->setParent(placeHolderParent);
        m_rectRubberBand->show();
        m_lineRubberBand->hide();
    } else if (suggestion.isReference() &&
               suggestion.referenceType() != QToolWindowManager::ReferenceAddTo) {
        QWidget *placeHolderParent;
        if (suggestion.widget()->topLevelWidget() == q->topLevelWidget())
            placeHolderParent = q;
        else
            placeHolderParent = suggestion.widget()->topLevelWidget();
        QRect placeHolderGeometry = sidePlaceHolderRect(suggestion.widget(),
                                                           suggestion.referenceType());
        placeHolderGeometry.moveTopLeft(suggestion.widget()->mapTo(placeHolderParent,
                                                                   placeHolderGeometry.topLeft()));

        m_lineRubberBand->setGeometry(placeHolderGeometry);
        m_lineRubberBand->setParent(placeHolderParent);
        m_lineRubberBand->show();
        m_rectRubberBand->hide();
    } else {
        qWarning("unsupported suggestion type");
    }
}

void QToolWindowManagerPrivate::findSuggestions(QToolWindowManagerWrapper *wrapper)
{
    m_suggestions.clear();
    m_dropCurrentSuggestionIndex = -1;
    QPoint globalPos = QCursor::pos();
    QWidgetList candidates;
    foreach (QSplitter *splitter, wrapper->findChildren<QSplitter*>())
        candidates << splitter;
    foreach (QAbstractToolWindowManagerArea *area, m_areas)
        if (area->topLevelWidget() == wrapper->topLevelWidget())
            candidates << area;
    foreach (QWidget *widget, candidates) {
        QSplitter *splitter = qobject_cast<QSplitter*>(widget);
        QAbstractToolWindowManagerArea *area = qobject_cast<QAbstractToolWindowManagerArea *>(widget);
        if (!splitter && !area) {
            qWarning("unexpected widget type");
            continue;
        }
        QSplitter *parentSplitter = qobject_cast<QSplitter*>(widget->parentWidget());
        bool lastInSplitter = parentSplitter &&
                parentSplitter->indexOf(widget) == parentSplitter->count() - 1;

        QList<QToolWindowManager::ReferenceType> allowedSides;
        if (!splitter || splitter->orientation() == Qt::Vertical)
            allowedSides << QToolWindowManager::ReferenceLeftOf;
        if (!splitter || splitter->orientation() == Qt::Horizontal)
            allowedSides << QToolWindowManager::ReferenceTopOf;
        if (!parentSplitter || parentSplitter->orientation() == Qt::Vertical || lastInSplitter)
            if (!splitter || splitter->orientation() == Qt::Vertical)
                allowedSides << QToolWindowManager::ReferenceRightOf;
        if (!parentSplitter || parentSplitter->orientation() == Qt::Horizontal || lastInSplitter)
            if (!splitter || splitter->orientation() == Qt::Horizontal)
                allowedSides << QToolWindowManager::ReferenceBottomOf;
        foreach (QToolWindowManager::ReferenceType side, allowedSides)
            if (sideSensitiveArea(widget, side).contains(widget->mapFromGlobal(globalPos)))
                m_suggestions << QToolWindowManagerAreaReference(side, widget);
        if (area && area->rect().contains(area->mapFromGlobal(globalPos)))
            m_suggestions << QToolWindowManagerAreaReference(QToolWindowManager::ReferenceAddTo, area);
    }
    if (candidates.isEmpty())
        m_suggestions << QToolWindowManager::EmptySpaceArea;

    if (m_suggestions.isEmpty())
        handleNoSuggestions();
    else
        showNextDropSuggestion();
}

QRect QToolWindowManagerPrivate::sideSensitiveArea(QWidget *widget,
                                            QToolWindowManager::ReferenceType side)
{
    QRect widgetRect = widget->rect();
    if (side == QToolWindowManager::ReferenceTopOf)
        return QRect(QPoint(widgetRect.left(), widgetRect.top() - m_borderSensitivity),
                     QSize(widgetRect.width(), m_borderSensitivity * 2));
    if (side == QToolWindowManager::ReferenceLeftOf)
        return QRect(QPoint(widgetRect.left() - m_borderSensitivity, widgetRect.top()),
                     QSize(m_borderSensitivity * 2, widgetRect.height()));

    if (side == QToolWindowManager::ReferenceBottomOf)
        return QRect(QPoint(widgetRect.left(),
                            widgetRect.top() + widgetRect.height() - m_borderSensitivity),
                     QSize(widgetRect.width(), m_borderSensitivity * 2));
    if (side == QToolWindowManager::ReferenceRightOf)
        return QRect(QPoint(widgetRect.left() + widgetRect.width() - m_borderSensitivity,
                            widgetRect.top()),
                     QSize(m_borderSensitivity * 2, widgetRect.height()));

    qWarning("invalid side");
    return QRect();
}

QRect QToolWindowManagerPrivate::sidePlaceHolderRect(QWidget *widget,
                                              QToolWindowManager::ReferenceType side)
{
    QRect widgetRect = widget->rect();
    QSplitter *parentSplitter = qobject_cast<QSplitter*>(widget->parentWidget());
    if (parentSplitter && parentSplitter->indexOf(widget) > 0) {
        int delta = parentSplitter->handleWidth() / 2 + m_rubberBandLineWidth / 2;
        if (side == QToolWindowManager::ReferenceTopOf && parentSplitter->orientation() == Qt::Vertical)
            return QRect(QPoint(widgetRect.left(), widgetRect.top() - delta),
                         QSize(widgetRect.width(), m_rubberBandLineWidth));
        if (side == QToolWindowManager::ReferenceLeftOf && parentSplitter->orientation() == Qt::Horizontal)
            return QRect(QPoint(widgetRect.left() - delta, widgetRect.top()),
                         QSize(m_rubberBandLineWidth, widgetRect.height()));
    }
    if (side == QToolWindowManager::ReferenceTopOf)
        return QRect(QPoint(widgetRect.left(), widgetRect.top()),
                     QSize(widgetRect.width(), m_rubberBandLineWidth));
    if (side == QToolWindowManager::ReferenceLeftOf)
        return QRect(QPoint(widgetRect.left(), widgetRect.top()),
                     QSize(m_rubberBandLineWidth, widgetRect.height()));
    if (side == QToolWindowManager::ReferenceBottomOf)
        return QRect(QPoint(widgetRect.left(),
                            widgetRect.top() + widgetRect.height() - m_rubberBandLineWidth),
                     QSize(widgetRect.width(), m_rubberBandLineWidth));
    if (side == QToolWindowManager::ReferenceRightOf)
        return QRect(QPoint(widgetRect.left() + widgetRect.width() - m_rubberBandLineWidth,
                            widgetRect.top()),
                     QSize(m_rubberBandLineWidth, widgetRect.height()));
    qWarning("invalid side");
    return QRect();
}

void QToolWindowManagerPrivate::updateDragPosition()
{
    if (!dragInProgress())
        return;
    if (!(qApp->mouseButtons() & Qt::LeftButton)) {
        finishDrag();
        return;
    }

    QPoint pos = QCursor::pos();
    m_dragIndicator->move(pos + QPoint(1, 1));
    bool foundWrapper = false;

    QWidget *window = qApp->topLevelAt(pos);
    foreach (QToolWindowManagerWrapper *wrapper, m_wrappers) {
        if (wrapper->window() == window) {
            if (wrapper->rect().contains(wrapper->mapFromGlobal(pos))) {
                findSuggestions(wrapper);
                if (!m_suggestions.isEmpty()) {
                    //starting or restarting timer
                    if (m_dropSuggestionSwitchTimer.isActive())
                        m_dropSuggestionSwitchTimer.stop();
                    m_dropSuggestionSwitchTimer.start();
                    foundWrapper = true;
                }
            }
            break;
        }
    }
    if (!foundWrapper)
        handleNoSuggestions();
}

void QToolWindowManagerPrivate::finishDrag()
{
    if (!dragInProgress()) {
        qWarning("unexpected finishDrag");
        return;
    }
    if (m_suggestions.isEmpty()) {
        moveToolWindows(m_draggedToolWindows, QToolWindowManager::NewFloatingArea);
    } else {
        if (m_dropCurrentSuggestionIndex >= m_suggestions.count()) {
            qWarning("invalid m_dropCurrentSuggestionIndex");
            return;
        }
        QToolWindowManagerAreaReference suggestion = m_suggestions[m_dropCurrentSuggestionIndex];
        handleNoSuggestions();
        moveToolWindows(m_draggedToolWindows, suggestion);
    }
    m_dragIndicator->hide();
    m_draggedToolWindows.clear();
}


QAbstractToolWindowManagerArea *QToolWindowManagerPrivate::createAndSetupArea() {
    Q_Q(QToolWindowManager);
    QAbstractToolWindowManagerArea *area = q->createArea();
    QObject::connect(area, SIGNAL(destroyed(QObject*)),
                     &slots_object, SLOT(areaDestroyed(QObject*)));
    m_areas << area;
    return area;
}

/*!
 * Create an area object. Reimplement this function if you want to use your own area subclass.
 */
QAbstractToolWindowManagerArea *QToolWindowManager::createArea() {
    return new QToolWindowManagerArea(this);
}

QSplitter *QToolWindowManagerPrivate::createAndSetupSplitter()
{
    Q_Q(QToolWindowManager);
    //currently no setup here
    return q->createSplitter();
}

QToolWindowManagerAreaReference::QToolWindowManagerAreaReference(QToolWindowManager::AreaType type)
{
    m_areaType = type;
    m_referenceType = static_cast<QToolWindowManager::ReferenceType>(-1);
    m_widget = 0;
}

QToolWindowManagerAreaReference::QToolWindowManagerAreaReference(QToolWindowManager::ReferenceType type,
                                                                 QWidget *widget)
{
    m_referenceType = type;
    m_areaType = static_cast<QToolWindowManager::AreaType>(-1);
    m_widget = widget;
    if ( !qobject_cast<QAbstractToolWindowManagerArea *>(widget) &&
         !qobject_cast<QSplitter*>(widget)) {
        qWarning("only QAbstractToolWindowManagerArea or splitter can be used with this type");
    }
}

/*!
  \reimp
*/
bool QToolWindowManager::event(QEvent *e)
{
    return QWidget::event(e);
}

void QToolWindowManagerPrivateSlots::showNextDropSuggestion()
{
    d->showNextDropSuggestion();
}

void QToolWindowManagerPrivateSlots::areaDestroyed(QObject *object)
{
    QAbstractToolWindowManagerArea *area = static_cast<QAbstractToolWindowManagerArea *>(object);
    if (area == d->m_lastUsedArea)
        d->m_lastUsedArea = 0;
    d->m_areas.removeOne(area);
}

void QToolWindowManagerAreaPrivateSlots::tabCloseRequested(int index)
{
    d->tabCloseRequested(index);
}
