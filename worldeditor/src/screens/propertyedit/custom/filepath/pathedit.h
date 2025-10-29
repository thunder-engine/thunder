#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class PathEdit;
}

class PathEdit : public PropertyEdit {
    Q_OBJECT
public:
    explicit PathEdit(bool file, QWidget *parent = nullptr);

    Variant data() const override;
    void setData(const Variant &data) override;

private slots:
    void onFileDialog();

    void onEditingFinished();

private:
    Ui::PathEdit *ui;

    TString m_path;

    bool m_file;

};

#endif // PATHEDIT_H
