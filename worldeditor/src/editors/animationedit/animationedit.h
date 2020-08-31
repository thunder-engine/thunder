#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

class AbstractSchemeModel;
class AnimationStateMachine;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    AnimationEdit();
    ~AnimationEdit();

    void readSettings();
    void writeSettings();

    void loadAsset(IConverterSettings *settings) override;

signals:
    void templateUpdate();

private slots:
    void on_actionSave_triggered();

    void onNodesSelected(const QVariant &);

    void onUpdateTemplate(bool update = true);

    void onToolWindowActionToggled(bool checked);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);

private:
    void closeEvent(QCloseEvent *event) override;
    bool isModified() const override;

    bool m_Modified;

    Ui::AnimationEdit *ui;

    AbstractSchemeModel *m_pBuilder;

    AnimationStateMachine *m_pMachine;

    QString m_Path;

    QAction *m_pUndo;
    QAction *m_pRedo;
};

#endif // ANIMATIONEDIT_H
