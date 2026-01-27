#ifndef UIEDIT_H
#define UIEDIT_H

#include <QMenu>

#include <editor/asseteditor.h>

#include <pugixml.hpp>

class WidgetController;
class UndoCommand;
class Widget;
class UiLoader;

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

    void onObjectCreate(TString type) override;
    void onObjectsSelected(Object::ObjectList objects, bool force) override;
    void onObjectsDeleted(Object::ObjectList objects) override;
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

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

    bool isModified() const override;

    StringList suffixes() const override;
    StringList componentGroups() const override;

    void saveElementHelper(pugi::xml_node &parent, Widget *widget);

    QAction *createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut = 0);

private:
    std::map<TString, Widget *> m_widgets;

    Ui::UiEdit *ui;

    QMenu m_widgetMenu;

    World *m_world;

    Scene *m_scene;

    UiLoader *m_loader;

    WidgetController *m_controller;

};

#endif // UIEDIT_H
