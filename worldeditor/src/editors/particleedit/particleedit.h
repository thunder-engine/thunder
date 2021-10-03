#ifndef PARTICLEEDIT_H
#define PARTICLEEDIT_H

#include <stdint.h>

#include <editor/asseteditor.h>

class Actor;
class ParticleRender;

class EffectConverter;

namespace Ui {
    class ParticleEdit;
}

class ParticleEdit : public AssetEditor {
    Q_OBJECT

public:
    ParticleEdit();
    ~ParticleEdit();

private slots:
    void onUpdateTemplate(bool update = true);

    void onNodeSelected(void *node);
    void onNodeDeleted();

    void onEmitterSelected(QString emitter);
    void onEmitterCreated();
    void onEmitterDeleted(QString name);

    void onFunctionSelected(QString emitter, QString function);
    void onFunctionCreated(QString emitter, QString function);
    void onFunctionDeleted(QString emitter, QString function);

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    bool isModified() const override;

    QStringList suffixes() const override;

    void timerEvent(QTimerEvent *) override;
    void changeEvent(QEvent *event) override;

    bool m_Modified;

    Ui::ParticleEdit *ui;

    Actor *m_pEffect;

    EffectConverter *m_pBuilder;

    ParticleRender *m_pRender;
};

#endif // PARTICLEEDIT_H
