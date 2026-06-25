#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QDialog>

#include <astring.h>

namespace Ui {
    class ProjectBrowser;
}

class ProjectBrowser : public QDialog {
    Q_OBJECT

public:
    explicit ProjectBrowser(QWidget *parent = nullptr);
    ~ProjectBrowser();

    TString projectPath() const { return m_projectPath; }

private:
    void onNewProject();
    void onImportProject();
    void onOpenProject();

    void onProjectSelected(const QModelIndex &index);

    void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::ProjectBrowser *ui;

    TString m_projectPath;
};

#endif // PROJECTBROWSER_H
