#ifndef UIEDIT_H
#define UIEDIT_H

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

private slots:
    void onActivated() override;

    void onUpdated() override;

    void onObjectCreate(QString type) override;
    void onObjectsSelected(std::list<Object *> objects, bool force) override;
    void onObjectsDeleted(std::list<Object *> objects) override;
    void onObjectsChanged(const std::list<Object *> &objects, QString property, const Variant &value) override;

    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    void changeEvent(QEvent *event) override;

    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    bool isModified() const override;

    StringList suffixes() const override;
    StringList componentGroups() const override;

    void saveElementHelper(pugi::xml_node &parent, Widget *widget);

    TString propertyTag(const MetaProperty &property, const TString &tag) const;

private:
    std::map<TString, Widget *> m_widgets;

    Ui::UiEdit *ui;

    World *m_world;

    Scene *m_scene;

    UiLoader *m_loader;

    WidgetController *m_controller;

    const UndoCommand *m_lastCommand;

};

#endif // UIEDIT_H
