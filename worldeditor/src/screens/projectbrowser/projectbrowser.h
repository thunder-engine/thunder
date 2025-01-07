#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QDialog>

namespace Ui {
    class ProjectBrowser;
}

class ProjectBrowser : public QDialog {
    Q_OBJECT

public:
    explicit ProjectBrowser(QWidget *parent = nullptr);
    ~ProjectBrowser();

    QString projectPath() const { return m_projectPath; }

private:
    void onNewProject();
    void onImportProject();
    void onOpenProject();

    void onProjectSelected(const QModelIndex &index);

    void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::ProjectBrowser *ui;

    QString m_projectPath;
};

#endif // PROJECTBROWSER_H
