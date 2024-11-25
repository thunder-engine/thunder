#ifndef COMMITREVERT_H
#define COMMITREVERT_H

#include <QWidget>

namespace Ui {
    class CommitRevert;
}

class CommitRevert : public QWidget {
    Q_OBJECT

public:
    explicit CommitRevert(QWidget *parent = nullptr);
    ~CommitRevert();

    void setObject(QObject *object);

signals:
    void reverted();

private slots:
    void onSettingsUpdated();

    void on_commitButton_clicked();
    void on_revertButton_clicked();

private:
    Ui::CommitRevert *ui;

    QObject *m_propertyObject;
};

#endif // COMMITREVERT_H
