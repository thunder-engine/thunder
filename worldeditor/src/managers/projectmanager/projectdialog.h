#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <QDialog>

namespace Ui {
    class ProjectDialog;
}

class ProjectModel;

class ProjectDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProjectDialog  (QWidget *parent = 0);
    ~ProjectDialog          ();

    static QString          projectPath     ();

    QString                 path            () const;

private slots:
    void                    on_listView_doubleClicked   (const QModelIndex &index);

    void                    on_listView_clicked         (const QModelIndex &index);

    void                    on_importBtn_clicked        ();

    void                    on_loadBtn_clicked          ();

    void on_newBtn_clicked();

private:
    Ui::ProjectDialog      *ui;

    QString                 m_Path;

    ProjectModel           *m_pModel;
};

#endif // PROJECTDIALOG_H
