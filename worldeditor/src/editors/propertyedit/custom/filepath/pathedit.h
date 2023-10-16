#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <editor/propertyedit.h>
#include <QFileInfo>

namespace Ui {
    class PathEdit;
}

class PathEdit : public PropertyEdit {
    Q_OBJECT
public:
    explicit PathEdit(QWidget *parent = nullptr);

    QVariant data() const;
    void setData(const QVariant &data);

private slots:
    void onFileDialog ();

private:
    Ui::PathEdit *ui;

    QFileInfo m_info;

};

#endif // PATHEDIT_H
