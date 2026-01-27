#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "nextmodel.h"
#include "property.h"
#include "propertydelegate.h"
#include "propertyfilter.h"

#include "editors/BooleanEdit.h"
#include "editors/IntegerEdit.h"
#include "editors/FloatEdit.h"
#include "editors/StringEdit.h"

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
#include "editor/pluginmanager.h"

PropertyEdit *createStandardEditor(int userType, QWidget *parent, const TString &editor) {
    switch(userType) {
        case MetaType::BOOLEAN: return new BooleanEdit(parent);
        case MetaType::INTEGER: return new IntegerEdit(parent);
        case MetaType::FLOAT: return new FloatEdit(parent);
        case MetaType::STRING: return new StringEdit(parent);
        case MetaType::VECTOR2:
        case MetaType::VECTOR3:
        case MetaType::VECTOR4: return new Vector4Edit(parent);
        default: break;
    }

    return nullptr;
}

PropertyEdit *createCustomEditor(int userType, QWidget *parent, const TString &editor) {
    if(userType == MetaType::VARIANTLIST) return new ArrayEdit(parent);

    if(editor == "Enum") return new NextEnumEdit(parent);
    else if(editor == "Path") return new PathEdit(false, parent);
    else if(editor == "FilePath") return new PathEdit(true, parent);
    else if(editor == "Locale") return new LocaleEdit(parent);
    else if(editor == "Axises") return new AxisesEdit(parent);
    else if(editor == "Alignment") return new AlignmentEdit(parent);
    else if(editor == "Color") return new ColorEdit(parent);
    else if(editor == "Asset" || editor == "Component") return new ObjectSelect(parent);

    return nullptr;
}

PropertyEditor::PropertyEditor(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::PropertyEditor),
        m_filter(new PropertyFilter(this)),
        m_editor(nullptr),
        m_topWidget(nullptr),
        m_nextModel(new NextModel(this)),
        m_item(nullptr) {

    ui->setupUi(this);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    PropertyEdit::registerEditorFactory(createCustomEditor);
    PropertyEdit::registerEditorFactory(createStandardEditor);

    connect(m_nextModel, &NextModel::propertyChanged, this, &PropertyEditor::objectsChanged);
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

void PropertyEditor::onObjectsSelected(const Object::ObjectList &objects) {
    m_nextModel->clear();
    m_filter->setSourceModel(m_nextModel);

    if(!objects.empty()) {
        m_item = objects.front();

        ContentBrowser *browser = dynamic_cast<ContentBrowser *>(sender());
        if(browser) {
            QWidget *widget = browser->commitRevert();
            connect(widget, SIGNAL(reverted()), this, SLOT(onUpdated()), Qt::UniqueConnection);
            setTopWidget(widget);
        } else if(m_editor) {
            setTopWidget(m_editor->propertiesWidget());
        }

        m_nextModel->addItem(m_item);
        for(auto it : m_item->getChildren()) {
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
            m_topWidget->setVisible(false);
        }

        m_topWidget = widget;
        if(m_topWidget) {
            m_topWidget->setParent(this);
            m_topWidget->setVisible(true);
            ui->verticalLayout->insertWidget(1, m_topWidget);
        }
    }
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

void PropertyEditor::onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) {

}

AssetEditor *PropertyEditor::currentEditor() const {
    return m_editor;
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

void PropertyEditor::on_treeView_customContextMenuRequested(const QPoint &pos) {
    if(m_editor) {
        QModelIndex index = ui->treeView->indexAt(pos);

        Property *p = static_cast<Property *>(m_filter->mapToSource(index).internalPointer());
        QMenu *menu = m_editor->propertyContextMenu(m_item, p->name());
        if(menu) {
            menu->exec(static_cast<QWidget*>(QObject::sender())->mapToGlobal(pos));
        }
    }
}

void PropertyEditor::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        QString title = windowTitle();

        ui->retranslateUi(this);

        setWindowTitle(title);
    }
}
