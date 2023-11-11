#include "propertyeditor.h"

#include "ui_propertyeditor.h"

#include "propertymodel.h"
#include "nextobject.h"

#include <editor/property.h>

#include "editors/BooleanEdit.h"
#include "editors/IntegerEdit.h"
#include "editors/FloatEdit.h"
#include "editors/StringEdit.h"
#include "editors/EnumEdit.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSignalMapper>
#include <QWidgetAction>
#include <QMenu>

#include "screens/componentbrowser/componentbrowser.h"
#include "editor/assetmanager.h"
#include "editor/asseteditor.h"
#include "editor/projectmanager.h"
#include "editor/settingsmanager.h"

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

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override {
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

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
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

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        const QSortFilterProxyModel *filter = static_cast<const QSortFilterProxyModel *>(model);
        QModelIndex origin = filter->mapToSource(index);
        QVariant data = static_cast<Property *>(origin.internalPointer())->editorData(editor);
        if(data.isValid()) {
            filter->sourceModel()->setData(origin, data, Qt::EditRole);
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
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
            while((pos = rx.indexIn(editorHints, pos)) != -1) {
                editor->setProperty(qPrintable(rx.cap(1).trimmed()), rx.cap(3).trimmed());
                pos += rx.matchedLength();
            }
            editor->blockSignals(false);
        }
    }

    QSignalMapper *m_finishedMapper;
};

PropertyEditor::PropertyEditor(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::PropertyEditor),
        m_filter(new PropertyFilter(this)),
        m_propertyObject(nullptr),
        m_nextObject(new NextObject(this)),
        m_editor(nullptr) {

    ui->setupUi(this);

    connect(m_nextObject, &NextObject::updated, this, &PropertyEditor::onUpdated);
    connect(m_nextObject, &NextObject::structureChanged, this, &PropertyEditor::onStructureChanged);

    m_filter->setSourceModel(new PropertyModel(this));

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    PropertyEdit::registerEditorFactory(createCustomEditor);

    ui->componentButton->setProperty("blue", true);
    ui->commitButton->setProperty("green", true);

    ComponentBrowser *comp = new ComponentBrowser(this);
    comp->setGroups({"Components"});

    QMenu *menu = new QMenu(ui->componentButton);
    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(comp);
    menu->addAction(action);
    ui->componentButton->setMenu(menu);

    connect(comp, &ComponentBrowser::componentSelected, m_nextObject, &NextObject::onCreateComponent);
    connect(comp, SIGNAL(componentSelected(QString)), menu, SLOT(hide()));
}

PropertyEditor::~PropertyEditor() {
    delete m_filter;
    delete m_nextObject;
    delete ui;

}

void PropertyEditor::addObject(QObject *propertyObject, const QString &name, QObject *parent) {
    if(propertyObject) {
        QAbstractItemModel *m = m_filter->sourceModel();
        static_cast<PropertyModel *>(m)->addItem(propertyObject, name, parent);
        ui->treeView->expandToDepth(-1);

        int i = 0;
        QModelIndex it = m->index(i, 1);
        while(it.isValid()) {
            updatePersistent(it);
            i++;
            it = m->index(i, 1);
        }
        ui->treeView->expandToDepth(-1);
    }
}

QObject *PropertyEditor::object() const {
    return m_propertyObject;
}

void PropertyEditor::onItemsSelected(QList<QObject *> items) {
    if(!items.empty()) {
        QObject *item = items.front();

        bool isCommitVisible = false;

        AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
        if(settings && settings != item) {
            AssetManager::instance()->checkImportSettings(settings);
            disconnect(settings, &AssetConverterSettings::updated, this, &PropertyEditor::onSettingsUpdated);

            disconnect(this, &PropertyEditor::reverted, settings, &AssetConverterSettings::loadSettings);
        } else {
            ProjectManager *projectManager = dynamic_cast<ProjectManager *>(m_propertyObject);
            if(projectManager && projectManager != item) {
                disconnect(projectManager, &ProjectManager::updated, this, &PropertyEditor::onSettingsUpdated);

                disconnect(this, &PropertyEditor::commited, projectManager, &ProjectManager::saveSettings);
                disconnect(this, &PropertyEditor::reverted, projectManager, &ProjectManager::loadSettings);
            } else {
                SettingsManager *settingsManager = dynamic_cast<SettingsManager *>(item);
                if(settingsManager) {
                    disconnect(settingsManager, &SettingsManager::updated, this, &PropertyEditor::onSettingsUpdated);

                    disconnect(this, &PropertyEditor::commited, SettingsManager::instance(), &SettingsManager::saveSettings);
                    disconnect(this, &PropertyEditor::reverted, SettingsManager::instance(), &SettingsManager::loadSettings);
                }
            }
        }

        settings = dynamic_cast<AssetConverterSettings *>(item);
        if(settings) {
            connect(settings, &AssetConverterSettings::updated, this, &PropertyEditor::onSettingsUpdated, Qt::UniqueConnection);

            connect(this, &PropertyEditor::reverted, settings, &AssetConverterSettings::loadSettings, Qt::UniqueConnection);

            ui->commitButton->setEnabled(settings->isModified());
            ui->revertButton->setEnabled(settings->isModified());

            isCommitVisible = true;
        } else {
            ProjectManager *projectManager = dynamic_cast<ProjectManager *>(item);
            if(projectManager) {
                connect(projectManager, &ProjectManager::updated, this, &PropertyEditor::onSettingsUpdated, Qt::UniqueConnection);

                connect(this, &PropertyEditor::commited, ProjectManager::instance(), &ProjectManager::saveSettings);
                connect(this, &PropertyEditor::reverted, ProjectManager::instance(), &ProjectManager::loadSettings);

                isCommitVisible = true;
            } else {
                SettingsManager *settingsManager = dynamic_cast<SettingsManager *>(item);
                if(settingsManager) {
                    connect(settingsManager, &SettingsManager::updated, this, &PropertyEditor::onSettingsUpdated, Qt::UniqueConnection);

                    connect(this, &PropertyEditor::commited, SettingsManager::instance(), &SettingsManager::saveSettings);
                    connect(this, &PropertyEditor::reverted, SettingsManager::instance(), &SettingsManager::loadSettings);

                    isCommitVisible = true;
                }
            }
        }

        ui->commitButton->setVisible(isCommitVisible);
        ui->revertButton->setVisible(isCommitVisible);

        ui->componentButton->setVisible(false);

        static_cast<PropertyModel *>(m_filter->sourceModel())->clear();

        addObject(item);
        m_propertyObject = item;
    } else {
        addObject(nullptr);
        m_propertyObject = nullptr;
    }
}

void PropertyEditor::onObjectsSelected(QList<Object *> objects) {
    ui->commitButton->setVisible(false);
    ui->revertButton->setVisible(false);

    static_cast<PropertyModel *>(m_filter->sourceModel())->clear();

    if(!objects.empty()) {
        ui->componentButton->setVisible(true);

        m_nextObject->setObject(objects.first());

        m_propertyObject = m_nextObject;
        addObject(m_propertyObject);
    } else {
        ui->componentButton->setVisible(false);

        m_propertyObject = nullptr;
    }
}

void PropertyEditor::onUpdated() {
    if(m_propertyObject == m_nextObject) {
        m_nextObject->onUpdated();
    } else {
        onSettingsUpdated();
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
    static_cast<PropertyModel *>(m_filter->sourceModel())->clear();

    if(m_propertyObject == m_nextObject) {
        m_nextObject->onUpdated();
    }

    addObject(m_propertyObject);
}

void PropertyEditor::onSettingsUpdated() {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings) {
        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    } else {
        ui->commitButton->setEnabled(true);
        ui->revertButton->setEnabled(true);
    }
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

        if(p->isPersistent()) {
            if(!ui->treeView->isPersistentEditorOpen(origin)) {
                ui->treeView->openPersistentEditor(origin);
            }

            QWidget *e = p->editor();
            if(e) {
                ui->treeView->itemDelegate()->setEditorData(e, origin);
            }
        }

        if(p->isRoot()) {
            ui->treeView->setFirstColumnSpanned(origin.row(), origin.parent(), true);
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
    if(false) { // Need to fetch available options
        QModelIndex origin = m_filter->mapToSource(ui->treeView->indexAt(pos));
        if(origin.isValid()) {
            PropertyModel *model = static_cast<PropertyModel *>(m_filter->sourceModel());
            QModelIndex index = model->index(origin.row(), 1, origin.parent());
            Property *item = static_cast<Property *>(index.internalPointer());

            m_nextObject->onPropertyContextMenuRequested(item->objectName(), ui->treeView->mapToGlobal(pos));
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

void PropertyEditor::on_commitButton_clicked() {
    AssetConverterSettings *s = dynamic_cast<AssetConverterSettings *>(object());
    if(s && s->isModified()) {
        s->saveSettings();
        AssetManager::instance()->pushToImport(s);
        AssetManager::instance()->reimport();
    }

    emit commited();

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void PropertyEditor::on_revertButton_clicked() {
    emit reverted();

    onUpdated();

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}
