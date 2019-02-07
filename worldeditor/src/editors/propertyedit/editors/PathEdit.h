#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QFileInfo>

class PathEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit PathEdit   (QWidget *parent = 0);

signals:
    void                pathChanged         (const QFileInfo &info);

private slots:
    void                onFileDialog        ();

private:
    void                resizeEvent         (QResizeEvent *event);

    QToolButton        *mToolBtn;
};

#endif // PATHEDIT_H
