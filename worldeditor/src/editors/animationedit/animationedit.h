#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include <editor/asseteditor.h>

class AbstractSchemeModel;
class AnimationStateMachine;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public AssetEditor {
    Q_OBJECT

public:
    AnimationEdit();
    ~AnimationEdit();

private slots:
    void onNodesSelected(const QVariant &);

    void onUpdateAsset(bool update = true);

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;

    QStringList suffixes() const override;

    void changeEvent(QEvent *event) override;

    bool m_Modified;

    Ui::AnimationEdit *ui;

    AbstractSchemeModel *m_pBuilder;

    AnimationStateMachine *m_pMachine;

};

#endif // ANIMATIONEDIT_H
