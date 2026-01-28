#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>

namespace Ui {
    class RenameDialog;
}

class RenameDialog : public QDialog {
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = nullptr);
    ~RenameDialog();

    QString text() const;
    void setText(const QString &text);

private slots:
    void on_okButton_clicked();

private:
    Ui::RenameDialog *ui;
};

#endif // RENAMEDIALOG_H
