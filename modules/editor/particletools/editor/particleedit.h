#ifndef PARTICLEEDIT_H
#define PARTICLEEDIT_H

#include <stdint.h>

#include <editor/asseteditor.h>

class Actor;
class EffectRender;

class EffectBuilder;

class CameraController;

class UndoCommand;

class QToolButton;

namespace Ui {
    class ParticleEdit;
}

class ParticleEdit : public AssetEditor {
    Q_OBJECT

public:
    ParticleEdit();
    ~ParticleEdit();

private slots:
    void onUpdateTemplate();

    void onDeleteModule();

    void onActivated() override;

    void onAddModule(QAction *action);

private:
    void readSettings();
    void writeSettings();

    std::list<QWidget *> createActionWidgets(QObject *object, QWidget *parent) const override;

    QWidget *propertiesWidget() override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    bool isModified() const override;

    QStringList suffixes() const override;

    void timerEvent(QTimerEvent *) override;
    void changeEvent(QEvent *event) override;

    Ui::ParticleEdit *ui;

    EffectBuilder *m_builder;

    CameraController *m_controller;

    Actor *m_effect;
    EffectRender *m_render;

    const UndoCommand *m_lastCommand;

    QToolButton *m_moduleButton;

};

#endif // PARTICLEEDIT_H
