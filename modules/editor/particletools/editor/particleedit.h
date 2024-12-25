#ifndef PARTICLEEDIT_H
#define PARTICLEEDIT_H

#include <stdint.h>

#include <editor/asseteditor.h>

class Actor;
class EffectRender;

class EffectBuilder;

class CameraController;

class UndoCommand;

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

private:
    void readSettings();
    void writeSettings();

    QList<QWidget *> createActionWidgets(QObject *object, QWidget *parent) const override;

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

};

#endif // PARTICLEEDIT_H
