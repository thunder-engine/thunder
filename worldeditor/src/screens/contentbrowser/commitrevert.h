#ifndef COMMITREVERT_H
#define COMMITREVERT_H

#include <QWidget>

#include <object.h>

namespace Ui {
    class CommitRevert;
}

class CommitRevert : public QWidget {
    Q_OBJECT

public:
    explicit CommitRevert(QWidget *parent = nullptr);
    ~CommitRevert();

    void setObject(Object *object);

signals:
    void reverted();

private slots:
    void onSettingsUpdated();

    void on_commitButton_clicked();
    void on_revertButton_clicked();

private:
    Ui::CommitRevert *ui;

    Object *m_propertyObject;
};

#endif // COMMITREVERT_H
