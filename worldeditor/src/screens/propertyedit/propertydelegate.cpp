/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "propertydelegate.h"

#include <QSignalMapper>
#include <QSortFilterProxyModel>

#include "property.h"
#include "nextmodel.h"

PropertyDelegate::PropertyDelegate(QObject *parent) :
        QStyledItemDelegate(parent),
        m_finishedMapper(new QSignalMapper(this)) {

}

PropertyDelegate::~PropertyDelegate() {
    delete m_finishedMapper;
}

QWidget *PropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const{
    QWidget *editor = nullptr;
    if(index.isValid()) {
        const QSortFilterProxyModel *model = static_cast<const QSortFilterProxyModel *>(index.model());
        NextModel *nextModel = static_cast<NextModel *>(model->sourceModel());
        QModelIndex origin = model->mapToSource(index);

        Property *p = static_cast<Property *>(nextModel->getObject(origin));
        if(p) {
            editor = p->getEditor(parent);
            if(editor) {
                if(editor->metaObject()->indexOfSignal("editFinished()") != -1) {
                    connect(editor, SIGNAL(editFinished()), m_finishedMapper, SLOT(map()));
                    m_finishedMapper->setMapping(editor, editor);
                }
            }
            parseEditorHints(editor, p->editorHints().data());
        }
    }

    return editor;
}

void PropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    m_finishedMapper->blockSignals(true);
    if(index.isValid()) {
        const QSortFilterProxyModel *model = static_cast<const QSortFilterProxyModel *>(index.model());
        NextModel *nextModel = static_cast<NextModel *>(model->sourceModel());
        QModelIndex origin = model->mapToSource(index);

        Property *p = static_cast<Property *>(nextModel->getObject(origin));
        if(p) {
            p->updateEditor();
        }
    }
    m_finishedMapper->blockSignals(false);
}

QSize PropertyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const QSortFilterProxyModel *model = static_cast<const QSortFilterProxyModel *>(index.model());
    NextModel *nextModel = static_cast<NextModel *>(model->sourceModel());
    QModelIndex origin = model->mapToSource(index);

    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(origin.isValid()) {
        Property *p = static_cast<Property *>(nextModel->getObject(origin));
        if(p) {
            result = p->sizeHint(result);
        }
    }
    return result;
}

void PropertyDelegate::parseEditorHints(QWidget *editor, const QString &editorHints) const {
    if(editor && !editorHints.isEmpty()) {
        editor->blockSignals(true);
        // Parse for property values
        static QRegularExpression rx("(.*)(=\\s*)(.*)(;{1})", QRegularExpression::InvertedGreedinessOption);
        auto it = rx.globalMatch(editorHints);
        while(it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            editor->setProperty(qPrintable(match.captured(1).trimmed()), match.captured(3).trimmed());
        }
        editor->blockSignals(false);
    }
}
