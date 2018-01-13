#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "propertymodel.h"
#include "custom/Property.h"

#include "nextobject.h"
#include "custom/BoolProperty.h"
#include "custom/StringProperty.h"
#include "custom/Vector2DProperty.h"
#include "custom/Vector3DProperty.h"
#include "custom/FilePathProperty.h"
#include "custom/AssetProperty.h"
#include "custom/ColorProperty.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSignalMapper>

Property *createCustomProperty(const QString &name, QObject *propertyObject, Property *parent) {
    int userType = 0;
    if(propertyObject) {
        userType    = propertyObject->property(qPrintable(name)).userType();
    }

    if(userType == QMetaType::Bool)
        return new BoolProperty(name, propertyObject, parent);

    if(userType == QMetaType::QString)
        return new StringProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Vector2"))
        return new Vector2DProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Vector3"))
        return new Vector3DProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("QColor"))
        return new ColorProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("FilePath"))
        return new FilePathProperty(name, propertyObject, parent);

    if(userType == QMetaType::type("Template"))
        return new TemplateProperty(name, propertyObject, parent);

    return 0;
}

class PropertyFilter : public QSortFilterProxyModel {
public:
    PropertyFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
    }

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
        QSortFilterProxyModel::sort(column, order);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model   = sourceModel();
        QModelIndex index           = model->index(sourceRow, 0, sourceParent);
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
    PropertyDelegate(QObject *parent = 0) :
        QStyledItemDelegate(parent) {

        m_finishedMapper    = new QSignalMapper(this);
        connect(m_finishedMapper, SIGNAL(mapped(QWidget*)), this, SIGNAL(commitData(QWidget*)));
        connect(m_finishedMapper, SIGNAL(mapped(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
    }

    virtual ~PropertyDelegate   () {
        delete m_finishedMapper;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QWidget *editor = nullptr;
        if(index.isValid()) {
            QModelIndex origin  = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
            Property *p = static_cast<Property *>(origin.internalPointer());
            editor      = p->createEditor(parent, option);
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

    virtual void PropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
        m_finishedMapper->blockSignals(true);
        QModelIndex origin  = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
        QVariant data   = origin.model()->data(origin, Qt::EditRole);

        if(!static_cast<Property *>(origin.internalPointer())->setEditorData(editor, data)) {
            QStyledItemDelegate::setEditorData(editor, index);
        }

        m_finishedMapper->blockSignals(false);
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
        const QSortFilterProxyModel *filter = static_cast<const QSortFilterProxyModel *>(model);
        QModelIndex origin  = filter->mapToSource(index);
        QVariant data = static_cast<Property *>(origin.internalPointer())->editorData(editor);
        if (data.isValid()) {
            filter->sourceModel()->setData(origin, data, Qt::EditRole);
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QModelIndex origin  = static_cast<const QSortFilterProxyModel *>(index.model())->mapToSource(index);
        QSize result    = QStyledItemDelegate::sizeHint(option, index);
        if(origin.isValid()) {
            Property *p     = static_cast<Property *>(origin.internalPointer());
            return p->sizeHint(result);
        }
        return result;
    }

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        if(index.data().type() == QMetaType::Bool) {
            //option->text    = "";
        }
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
        ui(new Ui::PropertyEditor) {

    ui->setupUi(this);

    m_pFilter   = new PropertyFilter(this);
    m_pFilter->setSourceModel(new PropertyModel(this));

    ui->treeView->setModel(m_pFilter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    registerCustomPropertyCB(createCustomProperty);
}

PropertyEditor::~PropertyEditor() {
    delete ui;
}

void PropertyEditor::addObject(QObject *propertyObject, const QString &name, QObject *parent) {
    if(propertyObject) {
        QAbstractItemModel *m   = m_pFilter->sourceModel();
        static_cast<PropertyModel *>(m)->addItem(propertyObject, name, parent);
        ui->treeView->expandToDepth(-1);

        int i   = 0;
        QModelIndex it  = m->index(i, 1);
        while(it.isValid()) {
            updatePersistent(it);
            i++;
            it  = m->index(i, 1);
        }
    }
}

void PropertyEditor::setObject(QObject *propertyObject) {
    clear();
    addObject(propertyObject);
}

void PropertyEditor::onUpdated() {
    QAbstractItemModel *m   = m_pFilter->sourceModel();
    static_cast<PropertyModel *>(m)->updateItem(nullptr);
    ui->treeView->expandToDepth(-1);

    int i   = 0;
    QModelIndex it  = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it  = m->index(i, 1);
    }
}

void PropertyEditor::registerCustomPropertyCB(UserTypeCB callback) {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->registerCustomPropertyCB(callback);
}

void PropertyEditor::unregisterCustomPropertyCB(UserTypeCB callback) {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->unregisterCustomPropertyCB(callback);
}

void PropertyEditor::clear() {
    static_cast<PropertyModel *>(m_pFilter->sourceModel())->clear();
}

void PropertyEditor::updatePersistent(const QModelIndex &index) {
    Property *p = static_cast<Property *>(index.internalPointer());
    if(p && p->isPersistent()) {
        ui->treeView->openPersistentEditor(m_pFilter->mapFromSource(index));

        QWidget *e  = p->editor();
        if(e) {
            ui->treeView->itemDelegate()->setEditorData(e, m_pFilter->mapFromSource(index));
        }
    }

    int i   = 0;
    QModelIndex it  = index.child(i, 1);
    while(it.isValid()) {
        updatePersistent(it);

        i++;
        it  = index.child(i, 1);
    }
}

void PropertyEditor::on_lineEdit_textChanged(const QString &arg1) {
    m_pFilter->setFilterFixedString(arg1);

    QAbstractItemModel *m   = m_pFilter->sourceModel();
    int i   = 0;
    QModelIndex it  = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it  = m->index(i, 1);
    }

    ui->treeView->expandToDepth(-1);
}

