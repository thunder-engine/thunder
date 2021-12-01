#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "propertymodel.h"
#include "nextobject.h"
#include "custom/Property.h"

#include "custom/BoolProperty.h"
#include "custom/IntegerProperty.h"
#include "custom/FloatProperty.h"
#include "custom/StringProperty.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSignalMapper>

Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent) {
    if(propertyObject == nullptr) {
        return nullptr;
    }

    QVariant value = propertyObject->property(qPrintable(name));
    switch(value.userType()) {
    case QMetaType::Bool: return new BoolProperty(name, propertyObject, parent);
    case QMetaType::Int: return new IntegerProperty(name, propertyObject, parent);
    case QMetaType::Float:
    case QMetaType::Double: return new FloatProperty(name, propertyObject, parent);
    case QMetaType::QString: return new StringProperty(name, propertyObject, parent);
    default: break;
    }

    return nullptr;
}

class PropertyFilter : public QSortFilterProxyModel {
public:
    explicit PropertyFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
    }

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
        QSortFilterProxyModel::sort(column, order);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 0, sourceParent);
        if(!filterRegExp().isEmpty() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(filterAcceptsRow(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegExp());
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
};

class PropertyDelegate : public QStyledItemDelegate {
public:
    explicit PropertyDelegate(QObject *parent = nullptr) :
        QStyledItemDelegate(parent) {

        m_finishedMapper = new QSignalMapper(this);
        connect(m_finishedMapper, SIGNAL(mapped(QWidget*)), this, SIGNAL(commitData(QWidget*)));
        connect(m_finishedMapper, SIGNAL(mapped(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
    }

    virtual ~PropertyDelegate() {
        delete m_finishedMapper;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const {
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
            parseEditorHints(editor, p->editorHints());
        }
        return editor;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const {
        m_finishedMapper->blockSignals(true);
        if(index.isValid()) {
            const QSortFilterProxyModel *model = static_cast<const QSortFilterProxyModel *>(index.model());
            QModelIndex origin = model->mapToSource(index);
            QVariant data = origin.model()->data(origin, Qt::EditRole);

            Property *p = static_cast<Property *>(origin.internalPointer());
            if(!p->setEditorData(editor, data)) {
                QStyledItemDelegate::setEditorData(editor, index);
            }
        }
        m_finishedMapper->blockSignals(false);
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
        const QSortFilterProxyModel *filter = static_cast<const QSortFilterProxyModel *>(model);
        QModelIndex origin = filter->mapToSource(index);
        QVariant data = static_cast<Property *>(origin.internalPointer())->editorData(editor);
        if(data.isValid()) {
            filter->sourceModel()->setData(origin, data, Qt::EditRole);
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QModelIndex origin = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
        QSize result = QStyledItemDelegate::sizeHint(option, index);
        if(origin.isValid()) {
            Property *p = static_cast<Property *>(origin.internalPointer());
            result = p->sizeHint(result);
        }
        return result;
    }

private:
    void parseEditorHints(QWidget *editor, const QString &editorHints) const {
        if(editor && !editorHints.isEmpty()) {
            editor->blockSignals(true);
            // Parse for property values
            QRegExp rx("(.*)(=\\s*)(.*)(;{1})");
            rx.setMinimal(true);
            int pos = 0;
            while ((pos = rx.indexIn(editorHints, pos)) != -1) {
                editor->setProperty(qPrintable(rx.cap(1).trimmed()), rx.cap(3).trimmed());
                pos += rx.matchedLength();
            }
            editor->blockSignals(false);
        }
    }

    QSignalMapper *m_finishedMapper;
};

PropertyEditor::PropertyEditor(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PropertyEditor),
        m_Animated(false),
        m_pPropertyObject(nullptr) {

    ui->setupUi(this);

    m_pFilter = new PropertyFilter(this);
    m_pFilter->setSourceModel(new PropertyModel(this));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_pFilter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    registerCustomPropertyCB(createCustomProperty);
    registerCustomPropertyCB(NextObject::createCustomProperty);
}

PropertyEditor::~PropertyEditor() {
    delete ui;
}

void PropertyEditor::addObject(QObject *propertyObject, const QString &name, QObject *parent) {
    if(propertyObject) {
        QAbstractItemModel *m = m_pFilter->sourceModel();
        static_cast<PropertyModel *>(m)->addItem(propertyObject, name, parent);
        ui->treeView->expandToDepth(-1);

        int i = 0;
        QModelIndex it = m->index(i, 1);
        while(it.isValid()) {
            updatePersistent(it);
            i++;
            it = m->index(i, 1);
        }

        if(propertyObject->metaObject()->indexOfSlot("onPropertyContextMenuRequested(QString,QPoint)") != -1) {
            connect(this, SIGNAL(propertyContextMenuRequested(QString,QPoint)), propertyObject, SLOT(onPropertyContextMenuRequested(QString,QPoint)));
        }
    }
}

QObject *PropertyEditor::object() const {
    return m_pPropertyObject;
}

void PropertyEditor::setObject(QObject *propertyObject) {
    clear();
    addObject(propertyObject);
    m_pPropertyObject = propertyObject;
}

void PropertyEditor::onUpdated() {
    QAbstractItemModel *m = m_pFilter->sourceModel();
    int i = 0;
    QModelIndex it = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m->index(i, 1);
    }
}

void PropertyEditor::onAnimated(bool flag) {
    m_Animated = flag;
}

void PropertyEditor::registerCustomPropertyCB(UserTypeCB callback) {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->registerCustomPropertyCB(callback);
}

void PropertyEditor::unregisterCustomPropertyCB(UserTypeCB callback) {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->unregisterCustomPropertyCB(callback);
}

void PropertyEditor::clear() {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->clear();
    if(m_pPropertyObject) {
        disconnect(this, &PropertyEditor::propertyContextMenuRequested, nullptr, nullptr);
    }
}

void PropertyEditor::updatePersistent(const QModelIndex &index) {
    Property *p = static_cast<Property *>(index.internalPointer());
    if(p && p->isPersistent()) {
        QModelIndex origin = m_pFilter->mapFromSource(index);
        if(!ui->treeView->isPersistentEditorOpen(origin)) {
            ui->treeView->openPersistentEditor(origin);
        }

        QWidget *e = p->editor();
        if(e) {
            ui->treeView->itemDelegate()->setEditorData(e, origin);
        }
    }

    int i = 0;
    QModelIndex it = index.child(i, 1);
    while(it.isValid()) {
        updatePersistent(it);

        i++;
        it = index.child(i, 1);
    }
}

void PropertyEditor::on_lineEdit_textChanged(const QString &arg1) {
    m_pFilter->setFilterFixedString(arg1);

    QAbstractItemModel *m = m_pFilter->sourceModel();
    int i = 0;
    QModelIndex it = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m->index(i, 1);
    }
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::on_treeView_customContextMenuRequested(const QPoint &pos) {
    if(m_Animated) {
        QModelIndex origin = m_pFilter->mapToSource(ui->treeView->indexAt(pos));
        if(origin.isValid()) {
            PropertyModel *model = static_cast<PropertyModel *>(m_pFilter->sourceModel());
            QModelIndex index = model->index(origin.row(), 1, origin.parent());
            Property *item = static_cast<Property *>(index.internalPointer());

            emit propertyContextMenuRequested(item->objectName(), ui->treeView->mapToGlobal(pos));
        }
    }
}

void PropertyEditor::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
