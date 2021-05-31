#ifndef PARTICLEEDIT_H
#define PARTICLEEDIT_H

#include <QMainWindow>
#include <stdint.h>

#include "editors/scenecomposer/documentmodel.h"

class QSettings;

class Engine;
class Actor;
class ParticleRender;

class Viewport;

class EffectConverter;

namespace Ui {
    class ParticleEdit;
}

class ParticleEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    ParticleEdit(DocumentModel *document);
    ~ParticleEdit();

    void readSettings();
    void writeSettings();

signals:
    void templateUpdate();

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

    void onToolWindowActionToggled(bool checked);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);

    void on_actionSave_triggered();

private:
    void loadAsset(IConverterSettings *settings) override;
    bool isModified() const override;

    QStringList assetTypes() const override;

    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *) override;

    bool m_Modified;

    Ui::ParticleEdit *ui;

    QObject *m_pEditor;

    Actor *m_pEffect;

    QString m_Path;

    EffectConverter *m_pBuilder;

    ParticleRender *m_pRender;

    DocumentModel *m_pDocument;
};

#endif // PARTICLEEDIT_H
