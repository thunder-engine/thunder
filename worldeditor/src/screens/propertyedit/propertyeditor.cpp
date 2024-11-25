#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "propertymodel.h"
#include "nextobject.h"
#include "property.h"
#include "propertydelegate.h"
#include "propertyfilter.h"

#include "editors/BooleanEdit.h"
#include "editors/IntegerEdit.h"
#include "editors/FloatEdit.h"
#include "editors/StringEdit.h"
#include "editors/EnumEdit.h"

#include "screens/contentbrowser/contentbrowser.h"
#include "editor/asseteditor.h"

PropertyEdit *createCustomEditor(int userType, QWidget *parent, const QString &, QObject *) {
    switch(userType) {
        case QMetaType::Bool: return new BooleanEdit(parent);
        case QMetaType::Int: return new IntegerEdit(parent);
        case QMetaType::Float:
        case QMetaType::Double: return new FloatEdit(parent);
        case QMetaType::QString: return new StringEdit(parent);
        case -1: return new EnumEdit(parent);
        default: break;
    }

    return nullptr;
}

PropertyEditor::PropertyEditor(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::PropertyEditor),
        m_filter(new PropertyFilter(this)),
        m_propertyObject(nullptr),
        m_nextObject(new NextObject(this)),
        m_editor(nullptr),
        m_topWidget(nullptr) {

    ui->setupUi(this);

    connect(m_nextObject, &NextObject::updated, this, &PropertyEditor::onUpdated);
    connect(m_nextObject, &NextObject::structureChanged, this, &PropertyEditor::onStructureChanged);
    connect(m_nextObject, &NextObject::structureChanged, this, &PropertyEditor::objectsSelected);

    m_filter->setSourceModel(new PropertyModel(this));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    PropertyEdit::registerEditorFactory(createCustomEditor);
}

PropertyEditor::~PropertyEditor() {
    delete m_filter;
    delete m_nextObject;
    delete ui;
}

void PropertyEditor::updateAndExpand() {
    QAbstractItemModel *model = m_filter->sourceModel();

    ui->treeView->expandToDepth(-1);
    int i = 0;
    QModelIndex it = model->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = model->index(i, 1);
    }
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::onItemsSelected(QList<QObject *> items) {
    if(!items.empty()) {
        QObject *item = items.front();

        ContentBrowser *browser = dynamic_cast<ContentBrowser *>(sender());
        if(browser) {
            QWidget *widget = browser->commitRevert();
            connect(widget, SIGNAL(reverted()), this, SLOT(onUpdated()), Qt::UniqueConnection);
            setTopWidget(widget);
        } else if(m_editor) {
            setTopWidget(m_editor->propertiesWidget());
        }

        PropertyModel *model = static_cast<PropertyModel *>(m_filter->sourceModel());
        model->clear();

        model->addItem(item);
        for(auto it : item->children()) {
            model->addItem(it, true);
        }

        updateAndExpand();

        m_propertyObject = item;
    } else {
        m_propertyObject = nullptr;

        setTopWidget(nullptr);
    }
}

void PropertyEditor::onObjectsSelected(QList<Object *> objects) {
    PropertyModel *model = static_cast<PropertyModel *>(m_filter->sourceModel());
    model->clear();

    if(!objects.empty()) {
        if(m_editor) {
            setTopWidget(m_editor->propertiesWidget());
        }

        m_nextObject->setObject(objects.first());

        m_propertyObject = m_nextObject;
        model->addItem(m_propertyObject);

        updateAndExpand();
    } else {
        m_propertyObject = nullptr;

        setTopWidget(nullptr);
    }
}

QAbstractItemModel *PropertyEditor::model() {
     return m_filter->sourceModel();
}

void PropertyEditor::setGroup(const QString &group) {
    m_filter->setGroup(group);
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::setTopWidget(QWidget *widget) {
    if(widget != m_topWidget) {
        if(m_topWidget) {
            ui->verticalLayout->removeWidget(m_topWidget);
            m_topWidget->setParent(nullptr);
        }

        m_topWidget = widget;
        if(m_topWidget) {
            ui->verticalLayout->insertWidget(1, m_topWidget);
        }
    }
}

QList<QWidget *> PropertyEditor::getActions(QObject *object, const QString &name, QWidget *parent) {
    NextObject *next = dynamic_cast<NextObject *>(m_propertyObject);
    if(next) {
        if(m_editor) {
            return m_editor->createActionWidgets(next->component(name), parent);
        }
    }

    return QList<QWidget *>();
}

void PropertyEditor::onUpdated() {
    if(m_propertyObject == m_nextObject) {
        m_nextObject->onUpdated();
    }
    QAbstractItemModel *m = m_filter->sourceModel();
    int i = 0;
    QModelIndex it = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m->index(i, 1);
        emit ui->treeView->itemDelegate()->sizeHintChanged(it);
    }
}

void PropertyEditor::onStructureChanged() {
    if(m_propertyObject == m_nextObject) {
        m_nextObject->onUpdated();
    }

    PropertyModel *model = static_cast<PropertyModel *>(m_filter->sourceModel());
    model->clear();

    model->addItem(m_propertyObject);

    updateAndExpand();
}

void PropertyEditor::onObjectsChanged(QList<Object *> objects, const QString property, Variant value) {

}

void PropertyEditor::setCurrentEditor(AssetEditor *editor) {
    if(m_editor) {
        disconnect(m_nextObject, &NextObject::propertyChanged, m_editor, &AssetEditor::onObjectsChanged);
    }

    m_editor = editor;
    connect(m_nextObject, &NextObject::propertyChanged, m_editor, &AssetEditor::onObjectsChanged);
}

void PropertyEditor::updatePersistent(const QModelIndex &index) {
    Property *p = static_cast<Property *>(index.internalPointer());
    if(p) {
        QModelIndex origin = m_filter->mapFromSource(index);

        if(!ui->treeView->isPersistentEditorOpen(origin)) {
            ui->treeView->openPersistentEditor(origin);
        }

        QWidget *e = p->editor();
        if(e) {
            ui->treeView->itemDelegate()->setEditorData(e, origin);
        }

        if(p->isRoot()) {
            ui->treeView->setFirstColumnSpanned(origin.row(), origin.parent(), true);
        }
    }

    int i = 0;
    QModelIndex it = index.model()->index(i, 1, index);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = index.model()->index(i, 1, index);
    }
}

void PropertyEditor::on_lineEdit_textChanged(const QString &arg1) {
    m_filter->setFilterFixedString(arg1);

    QAbstractItemModel *m = m_filter->sourceModel();
    int i = 0;
    QModelIndex it = m->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m->index(i, 1);
    }
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        QString title = windowTitle();

        ui->retranslateUi(this);

        setWindowTitle(title);
    }
}
