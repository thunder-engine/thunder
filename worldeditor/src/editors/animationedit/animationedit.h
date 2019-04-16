#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

class AnimationBuilder;
class AnimationStateMachine;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    explicit AnimationEdit(Engine *engine);
    ~AnimationEdit();

    void readSettings();
    void writeSettings();

    void loadAsset(IConverterSettings *settings);

signals:
    void templateUpdate();

private slots:
    void on_actionSave_triggered();

    void onNodeSelected(int);

    void onUpdateTemplate(bool update = true);

    void onToolWindowActionToggled(bool checked);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);

private:
    void closeEvent(QCloseEvent *event);

    Ui::AnimationEdit *ui;

    AnimationBuilder *m_pBuilder;

    AnimationStateMachine *m_pMachine;

    QString m_Path;
};

#endif // ANIMATIONEDIT_H
