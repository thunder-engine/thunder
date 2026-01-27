#include "propertydelegate.h"

#include <QSignalMapper>
#include <QSortFilterProxyModel>

#include "property.h"

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
        QModelIndex origin = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
        Property *p = static_cast<Property *>(origin.internalPointer());
        editor = p->getEditor(parent);
        if(editor) {
            if(editor->metaObject()->indexOfSignal("editFinished()") != -1) {
                connect(editor, SIGNAL(editFinished()), m_finishedMapper, SLOT(map()));
                m_finishedMapper->setMapping(editor, editor);
            }
        }
        parseEditorHints(editor, p->editorHints().data());
    }

    return editor;
}

void PropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    m_finishedMapper->blockSignals(true);
    if(index.isValid()) {
        const QSortFilterProxyModel *model = static_cast<const QSortFilterProxyModel *>(index.model());
        QModelIndex origin = model->mapToSource(index);
        Property *p = static_cast<Property *>(origin.internalPointer());
        p->updateEditor();
    }
    m_finishedMapper->blockSignals(false);
}

QSize PropertyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QModelIndex origin = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    if(origin.isValid()) {
        Property *p = static_cast<Property *>(origin.internalPointer());
        result = p->sizeHint(result);
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
