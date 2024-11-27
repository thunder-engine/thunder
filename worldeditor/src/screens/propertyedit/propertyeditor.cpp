#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "propertymodel.h"
#include "nextmodel.h"
#include "property.h"
#include "propertydelegate.h"
#include "propertyfilter.h"

#include "editors/BooleanEdit.h"
#include "editors/IntegerEdit.h"
#include "editors/FloatEdit.h"
#include "editors/StringEdit.h"
#include "editors/EnumEdit.h"

#include "custom/array/arrayedit.h"
#include "custom/alignment/alignmentedit.h"
#include "custom/axises/axisesedit.h"
#include "custom/color/coloredit.h"
#include "custom/locale/localeedit.h"
#include "custom/objectselect/objectselect.h"
#include "custom/filepath/pathedit.h"
#include "custom/nextenum/nextenumedit.h"
#include "custom/vector4/vector4edit.h"

#include "screens/contentbrowser/contentbrowser.h"
#include "editor/asseteditor.h"

PropertyEdit *createStandardEditor(int userType, QWidget *parent, const QString &, QObject *) {
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

PropertyEdit *createCustomEditor(int userType, QWidget *parent, const QString &name, QObject *object) {
    PropertyEdit *result = nullptr;

    if(userType == QMetaType::QVariantList) {
        result = new ArrayEdit(parent);
    } else if(userType == qMetaTypeId<Vector2>() ||
              userType == qMetaTypeId<Vector3>() ||
              userType == qMetaTypeId<Vector4>()) {

        result = new Vector4Edit(parent);
    } else if(userType == qMetaTypeId<Enum>()) {

        result = new NextEnumEdit(parent);
    } else if(userType == qMetaTypeId<QFileInfo>()) {

        result = new PathEdit(parent);
    } else if(userType == qMetaTypeId<QLocale>()) {

        result = new LocaleEdit(parent);
    } else if(userType == qMetaTypeId<Axises>()) {

        result = new AxisesEdit(parent);
    } else if(userType == qMetaTypeId<Alignment>()) {

        result = new AlignmentEdit(parent);
    } else if(userType == qMetaTypeId<QColor>()) {

        result = new ColorEdit(parent);
    } else if(userType == qMetaTypeId<Template>() ||
              userType == qMetaTypeId<ObjectData>()) {

        result = new ObjectSelect(parent);
    }

    return result;
}

PropertyEditor::PropertyEditor(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::PropertyEditor),
        m_filter(new PropertyFilter(this)),
        m_editor(nullptr),
        m_topWidget(nullptr),
        m_nextModel(new NextModel(this)),
        m_propertyModel(new PropertyModel(this)) {

    ui->setupUi(this);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    PropertyEdit::registerEditorFactory(createStandardEditor);
    PropertyEdit::registerEditorFactory(createCustomEditor);
}

PropertyEditor::~PropertyEditor() {
    delete m_filter;
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
    m_propertyModel->clear();
    m_filter->setSourceModel(m_propertyModel);

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

        m_propertyModel->addItem(item);
        for(auto it : item->children()) {
            m_propertyModel->addItem(it);
        }

        updateAndExpand();
    } else {
        setTopWidget(nullptr);
    }
}

void PropertyEditor::onObjectsSelected(QList<Object *> objects) {
    m_nextModel->clear();
    m_filter->setSourceModel(m_nextModel);

    if(!objects.empty()) {
        Object *item = objects.first();

        if(m_editor) {
            setTopWidget(m_editor->propertiesWidget());
        }

        m_nextModel->addItem(item);
        for(auto it : item->getChildren()) {
            if(dynamic_cast<Actor *>(it) == nullptr) {
                m_nextModel->addItem(it);
            }
        }

        updateAndExpand();
    } else {
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

QList<QWidget *> PropertyEditor::getActions(QObject *object, QWidget *parent) {
    if(m_editor) {
        return m_editor->createActionWidgets(object, parent);
    }

    return QList<QWidget *>();
}

QList<QWidget *> PropertyEditor::getActions(Object *object, QWidget *parent) {
    if(m_editor) {
        return m_editor->createActionWidgets(object, parent);
    }

    return QList<QWidget *>();
}

void PropertyEditor::onUpdated() {
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

void PropertyEditor::onObjectsChanged(QList<Object *> objects, const QString property, Variant value) {

}

void PropertyEditor::setCurrentEditor(AssetEditor *editor) {
    if(m_editor != editor) {
        if(m_editor) {
            disconnect(m_nextModel, &NextModel::propertyChanged, m_editor, &AssetEditor::onObjectsChanged);
        }

        m_editor = editor;

        connect(m_nextModel, &NextModel::propertyChanged, m_editor, &AssetEditor::onObjectsChanged);
    }
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
