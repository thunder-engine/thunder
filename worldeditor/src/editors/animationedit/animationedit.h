#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include "editors/scenecomposer/documentmodel.h"

class AbstractSchemeModel;
class AnimationStateMachine;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public QWidget, public IAssetEditor {
    Q_OBJECT

public:
    AnimationEdit(DocumentModel *document);
    ~AnimationEdit();

signals:
    void templateUpdate();

private slots:
    void onNodesSelected(const QVariant &);

    void onUpdateTemplate(bool update = true);

private:
    void loadAsset(IConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;

    QStringList assetTypes() const override;

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    bool m_Modified;

    Ui::AnimationEdit *ui;

    AbstractSchemeModel *m_pBuilder;

    AnimationStateMachine *m_pMachine;

    QString m_Path;

    DocumentModel *m_pDocument;
};

#endif // ANIMATIONEDIT_H
