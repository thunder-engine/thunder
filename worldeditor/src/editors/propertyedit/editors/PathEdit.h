#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <QWidget>
#include <QFileInfo>

namespace Ui {
    class PathEdit;
}

class PathEdit : public QWidget {
    Q_OBJECT
public:
    explicit PathEdit(QWidget *parent = nullptr);

    QFileInfo data() const;
    void setData(const QFileInfo &v);

signals:
    void pathChanged(const QFileInfo &info);

private slots:
    void onFileDialog();

private:
    Ui::PathEdit   *ui;

    QFileInfo       m_Info;
};

#endif // PATHEDIT_H
