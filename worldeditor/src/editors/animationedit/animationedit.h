#ifndef ANIMATIONEDIT_H
#define ANIMATIONEDIT_H

#include <QMainWindow>

#include <editor/asseteditor.h>

class AbstractSchemeModel;
class AnimationStateMachine;

class ComponentBrowser;

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

    void onShowContextMenu(int node, int port, bool out);

    void onComponentSelected(const QString &path);

    void onActivated() override;

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isModified() const override;

    QStringList suffixes() const override;

    void changeEvent(QEvent *event) override;

private:
    Ui::AnimationEdit *ui;

    AbstractSchemeModel *m_schemeModel;

    AnimationStateMachine *m_stateMachine;

    QMenu *m_createMenu;

    ComponentBrowser *m_browser;

    QObject *m_selectedItem;

    int m_node;
    int m_port;
    bool m_out;

    bool m_modified;

};

#endif // ANIMATIONEDIT_H
