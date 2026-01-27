#ifndef PARTICLEEDIT_H
#define PARTICLEEDIT_H


#include <editor/asseteditor.h>

class Actor;
class EffectRender;

class EffectBuilder;

class CameraController;

class UndoCommand;

class QToolButton;

class ParticleProxy;

namespace Ui {
    class ParticleEdit;
}

class ParticleEdit : public AssetEditor {
    Q_OBJECT

public:
    ParticleEdit();
    ~ParticleEdit();

    void onUpdateTemplate();
    void onModuleChanged();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onDeleteModule();

    void onActivated() override;

    void onAddModule(QAction *action);

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void readSettings();
    void writeSettings();

    std::list<QWidget *> propertiesActionWidgets(Object *object, QWidget *parent) const override;

    QWidget *propertiesWidget() override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    bool isModified() const override;

    StringList suffixes() const override;

    void timerEvent(QTimerEvent *) override;
    void changeEvent(QEvent *event) override;

    Ui::ParticleEdit *ui;

    EffectBuilder *m_builder;

    CameraController *m_controller;

    Actor *m_light;

    Actor *m_effect;
    EffectRender *m_render;

    QToolButton *m_moduleButton;

    ParticleProxy *m_proxy;

};

#endif // PARTICLEEDIT_H
