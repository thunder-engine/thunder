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
#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <editor/editorgadget.h>

class QAbstractItemModel;
class PropertyFilter;

class Object;
class NextModel;
class PropertyModel;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public EditorGadget {
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parent = nullptr);

    ~PropertyEditor();

    void onSelectionChanged() override;

    QAbstractItemModel *model();

    void setGroup(const QString &group);

    void setTopWidget(QWidget *widget);

    AssetEditor *currentEditor() const;
    void setCurrentEditor(AssetEditor *editor) override;

public slots:
    void onObjectSelected(Object *object);

protected:
    void updatePersistent(const QModelIndex &index);

    void updateAndExpand();

protected slots:
    void onUpdated() override;

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override;

    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    void changeEvent(QEvent *event) override;

private:
    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    AssetEditor *m_editor;

    QWidget *m_topWidget;

    NextModel *m_nextModel;

    Object *m_item;

};

#endif // PROPERTYEDITOR_H
