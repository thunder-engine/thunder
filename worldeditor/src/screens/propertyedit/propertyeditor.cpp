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
#include "editor/legacyasseteditor.h"
#include "editor/pluginmanager.h"

LegacyPropertyEdit *createStandardEditor(int userType, QWidget *parent, const TString &editor) {
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

LegacyPropertyEdit *createCustomEditor(int userType, QWidget *parent, const TString &editor) {
    if(userType == MetaType::VARIANTLIST) return new ArrayEdit(parent);

    if(editor == "Enum") return new NextEnumEdit(parent);
    else if(editor == "Path") return new PathEdit(false, parent);
    else if(editor == "FilePath") return new PathEdit(true, parent);
    else if(editor == "Locale") return new LocaleEdit(parent);
    else if(editor == "Axises") return new AxisesEdit(parent);
    else if(editor == "Alignment") return new AlignmentEdit(parent);
    else if(editor == "Color") return new ColorEdit(parent);
    else if(editor == "Asset" || editor == "Component") return new ObjectSelect(parent);

    for(auto &it : Editor::plugins()->extensions("property")) {
        if(it == editor) {
            LegacyPropertyEdit *edit = reinterpret_cast<LegacyPropertyEdit *>(Editor::plugins()->getPluginObject(it));
            if(edit) {
                edit->setParent(parent);
                return edit;
            }
        }
    }

    return nullptr;
}

PropertyEditor::PropertyEditor(QWidget *parent) :
        LegacyEditorGadget(parent),
        ui(new Ui::PropertyEditor),
        m_filter(new PropertyFilter(this)),
        m_topWidget(nullptr),
        m_nextModel(new NextModel(this)) {

    ui->setupUi(this);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new PropertyDelegate(this));

    LegacyPropertyEdit::registerEditorFactory(createCustomEditor);
    LegacyPropertyEdit::registerEditorFactory(createStandardEditor);

    connect(m_nextModel, &NextModel::propertyChanged, this, &PropertyEditor::onPropertyChanged);

    m_filter->setSourceModel(m_nextModel);
}

PropertyEditor::~PropertyEditor() {
    delete m_filter;
    delete ui;
}

void PropertyEditor::updateAndExpand() {
    ui->treeView->expandToDepth(-1);
    int i = 0;
    QModelIndex it = m_nextModel->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m_nextModel->index(i, 1);
    }
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::onSelectionChanged() {
    m_selectedObjects = m_currentEditor->selected();
    const Object::ObjectList selectedObjects = m_selectedObjects;
    bool maskPropertyType = m_currentEditor->maskPropertyType();
    if(!m_selectedObjects.empty()) {
        m_nextModel->clear();
        Object *item = m_selectedObjects.front();
        setTopWidget(m_currentEditor->propertiesWidget());
        m_nextModel->addObjects(m_selectedObjects, maskPropertyType);

        Actor *firstActor = dynamic_cast<Actor *>(m_selectedObjects.front());
        if(firstActor && m_selectedObjects.size() > 1) {
            for(Object *child : firstActor->getChildren()) {
                if(dynamic_cast<Actor *>(child) != nullptr) {
                    continue;
                }

                TString type = child->typeName();
                Object::ObjectList components = {child};
                bool presentOnAllActors = true;

                auto object = selectedObjects.begin();
                ++object;
                for(; object != selectedObjects.end(); ++object) {
                    Actor *actor = dynamic_cast<Actor *>(*object);
                    Component *component = actor ? actor->component(type) : nullptr;
                    if(component == nullptr) {
                        presentOnAllActors = false;
                        break;
                    }
                    components.push_back(component);
                }

                if(presentOnAllActors) {
                    m_nextModel->addObjects(components, maskPropertyType);

                    m_selectedObjects.insert(m_selectedObjects.end(), components.begin(), components.end());
                }
            }
        } else {
            for(auto it : item->getChildren()) {
                if(dynamic_cast<Actor *>(it) == nullptr) {
                    m_nextModel->addObject(it, maskPropertyType);
                    m_selectedObjects.push_back(it);
                }
            }
        }

        updateAndExpand();
    } else {
        m_nextModel->clear();
        setTopWidget(nullptr);
    }
}

void PropertyEditor::onObjectSelected(Object *object) {
    m_nextModel->clear();
    if(object) {
        ContentBrowser *browser = dynamic_cast<ContentBrowser *>(sender());
        if(browser) {
            QWidget *widget = browser->commitRevert();
            connect(widget, SIGNAL(reverted()), this, SLOT(onUpdated()), Qt::UniqueConnection);
            setTopWidget(widget);
        } else if(m_currentEditor) {
            setTopWidget(m_currentEditor->propertiesWidget());
        }

        m_nextModel->addObject(object);
        for(auto it : object->getChildren()) {
            if(dynamic_cast<Actor *>(it) == nullptr) {
                m_nextModel->addObject(it);
            }
        }

        updateAndExpand();
    } else {
        setTopWidget(nullptr);
    }
}

QAbstractItemModel *PropertyEditor::model() {
    return m_nextModel;
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
    int i = 0;
    QModelIndex it = m_nextModel->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m_nextModel->index(i, 1);
        emit ui->treeView->itemDelegate()->sizeHintChanged(it);
    }
}

void PropertyEditor::onPropertyChanged(const TString &property, const Variant &value) {
    emit objectsChanged(property, value);
}

void PropertyEditor::setCurrentEditor(LegacyAssetEditor *editor) {
    if(m_currentEditor != editor) {
        if(m_currentEditor) {
            disconnect(this, &PropertyEditor::objectsChanged, m_currentEditor, &LegacyAssetEditor::onObjectsChanged);
        }

        LegacyEditorGadget::setCurrentEditor(editor);

        connect(this, &PropertyEditor::objectsChanged, m_currentEditor, &LegacyAssetEditor::onObjectsChanged);
    }
}

Object::ObjectList PropertyEditor::selected() {
    return m_selectedObjects;
}

void PropertyEditor::updatePersistent(const QModelIndex &index) {
    QModelIndex origin = m_filter->mapFromSource(index);

    if(!ui->treeView->isPersistentEditorOpen(origin)) {
        ui->treeView->openPersistentEditor(origin);
    }

    Property *p = static_cast<Property *>(m_nextModel->getObject(index));
    if(p) {
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

    int i = 0;
    QModelIndex it = m_nextModel->index(i, 1);
    while(it.isValid()) {
        updatePersistent(it);
        i++;
        it = m_nextModel->index(i, 1);
    }
    ui->treeView->expandToDepth(-1);
}

void PropertyEditor::on_treeView_customContextMenuRequested(const QPoint &pos) {
    if(m_currentEditor) {
        QModelIndex index = ui->treeView->indexAt(pos);

        Property *p = static_cast<Property *>(m_filter->mapToSource(index).internalPointer());
        if(p && !m_selectedObjects.empty()) {
            QMenu *menu = m_currentEditor->propertyContextMenu(m_selectedObjects.front(), p->name());
            if(menu) {
                menu->exec(static_cast<QWidget*>(QObject::sender())->mapToGlobal(pos));
            }
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
