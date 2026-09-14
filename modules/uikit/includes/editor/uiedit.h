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
#ifndef UIEDIT_H
#define UIEDIT_H

#include <QMenu>

#include <editor/asseteditor.h>

#include <pugixml.hpp>

class UiLoader;
class WidgetController;

class Widget;
class Canvas;
class Scene;

namespace Ui {
    class UiEdit;
}

class UiEdit : public AssetEditor {
    Q_OBJECT
    
public:
    UiEdit();
    ~UiEdit();

    static TString propertyTag(const MetaProperty &property, const TString &tag);

private slots:
    void onActivated() override;

    void onUpdated() override;

    void onObjectCreate(const TString &type) override;
    void onObjectsSelected(Object::ObjectList objects, bool force) override;
    void onSelectionDeleted() override;
    void onObjectsChanged(const Object::ObjectList &objects, const TString &propertyName, const Variant &value) override;

    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onWidgetDelete();
    void onWidgetDuplicate();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    void changeEvent(QEvent *event) override;

    QMenu *hierarchyContextMenu(Object *object) override;

    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    StringList suffixes() const override;
    StringList componentGroups() const override;
    TString assetType() const override;

    Object::ObjectList selected() const override;

    void changeParent(const Object::ObjectList &objects, Object *parent, int position = -1) override;

    void saveElementHelper(pugi::xml_node &parent, Widget *widget);

    QAction *createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut = 0);

private:
    std::map<TString, Widget *> m_widgets;

    Ui::UiEdit *ui;

    QMenu m_widgetMenu;

    World *m_world;

    Scene *m_scene;

    Canvas *m_canvas;

    UiLoader *m_loader;

    WidgetController *m_controller;

};

#endif // UIEDIT_H
