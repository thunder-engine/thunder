#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include <editor/asseteditor.h>

class AbstractNodeGraph;
class AnimationStateMachine;
class AssetConverter;

class UndoCommand;

namespace Ui {
    class AnimationEdit;
}

class AnimationEdit : public AssetEditor {
    Q_OBJECT

public:
    AnimationEdit();
    ~AnimationEdit();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onObjectsChanged(const std::list<Object *> &objects, QString property, const Variant &value) override;

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;

    QStringList suffixes() const override;

    void changeEvent(QEvent *event) override;

private:
    Ui::AnimationEdit *ui;

    AbstractNodeGraph *m_graph;
    AssetConverter *m_assetConverter;

    AnimationStateMachine *m_stateMachine;

    const UndoCommand *m_lastCommand;

};

#endif // ANIMATIONEDIT_H
