#ifndef COMMITREVERT_H
#define COMMITREVERT_H

#include <QWidget>

#include <engine.h>

class CommitRevertProxy;

namespace Ui {
    class CommitRevert;
}

class CommitRevert : public QWidget {
    Q_OBJECT

public:
    explicit CommitRevert(QWidget *parent = nullptr);
    ~CommitRevert();

    void setObject(Object *object);

    void onSettingsUpdated();

signals:
    void reverted();

private slots:
    void on_commitButton_clicked();
    void on_revertButton_clicked();

private:
    Ui::CommitRevert *ui;

    Object *m_propertyObject;

    CommitRevertProxy *m_proxy;

};

class CommitRevertProxy : public Object {
    A_OBJECT(CommitRevertProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(CommitRevertProxy::onUpdated)
    )
public:
    void setEditor(CommitRevert *editor) {
        m_editor = editor;
    }

    void onUpdated() {
        m_editor->onSettingsUpdated();
    }

private:
    CommitRevert *m_editor = nullptr;

};

#endif // COMMITREVERT_H
