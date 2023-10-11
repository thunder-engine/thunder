#ifndef UIEDIT_H
#define UIEDIT_H

#include <editor/asseteditor.h>

class QDomElement;
class WidgetController;
class UndoCommand;
class Layout;

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
    void readSettings();
    void writeSettings();

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    void resizeEvent(QResizeEvent *event) override;

    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    QStringList suffixes() const override;
    QStringList componentGroups() const override;

private:
    Ui::UiEdit *ui;

    World *m_world;

    Scene *m_scene;

    Actor *m_screenActor;

    WidgetController *m_controller;

    const UndoCommand *m_lastCommand;

};

#endif // UIEDIT_H
