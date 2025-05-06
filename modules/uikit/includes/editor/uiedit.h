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
    void onObjectsSelected(QList<Object *> objects, bool force) override;
    void onObjectsDeleted(QList<Object *> objects) override;
    void onObjectsChanged(const QList<Object *> &objects, QString property, const Variant &value) override;

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    QStringList suffixes() const override;
    QStringList componentGroups() const override;

    void saveElementHelper(pugi::xml_node &parent, Widget *widget);

    std::string propertyTag(const MetaProperty &property, const std::string &tag) const;

private:
    std::map<std::string, Widget *> m_widgets;

    Ui::UiEdit *ui;

    World *m_world;

    Scene *m_scene;

    UiLoader *m_loader;

    WidgetController *m_controller;

    const UndoCommand *m_lastCommand;

};

#endif // UIEDIT_H
