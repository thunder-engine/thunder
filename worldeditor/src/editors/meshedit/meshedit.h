#ifndef MESHEDIT_H
#define MESHEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

class Engine;
class Actor;
class DirectLight;

class Viewport;
class NextObject;

namespace Ui {
    class MeshEdit;
}

class MeshEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    MeshEdit (DocumentModel *document);
    ~MeshEdit ();

    void readSettings ();
    void writeSettings ();

signals:
    void templateUpdate ();

private:
    void loadAsset (IConverterSettings *settings) override;
    bool isModified() const override;

    QStringList assetTypes() const override;

    void closeEvent (QCloseEvent *event) override;
    void timerEvent (QTimerEvent *) override;

    bool m_Modified;

    Ui::MeshEdit *ui;

    Actor *m_pMesh;
    Actor *m_pGround;
    Actor *m_pDome;
    Actor *m_pLight;

    QString m_Path;

    IConverterSettings *m_pSettings;

    Viewport *glWidget;

    DocumentModel *m_pDocument;

private slots:
    void onGLInit ();

    void onUpdateTemplate ();

    void onToolWindowActionToggled (bool checked);

    void onToolWindowVisibilityChanged (QWidget *toolWindow, bool visible);

    void on_actionSave_triggered ();

};

#endif // MESHEDIT_H
