#ifndef COMPONENTBROWSER_H
#define COMPONENTBROWSER_H

#include <QWidget>

class ComponentFilter;

namespace Ui {
    class ComponentBrowser;
}

class ComponentBrowser : public QWidget {
    Q_OBJECT
public:
    ComponentBrowser(QWidget *parent = 0);

    void setGroups(const QStringList &groups = QStringList());

signals:
    void componentSelected(const QString &uri);

private slots:
    void on_findComponent_textChanged(const QString &arg1);

    void on_componentsTree_clicked(const QModelIndex &index);

private:
    void changeEvent(QEvent *event) override;

    Ui::ComponentBrowser *ui;

    ComponentFilter *m_proxyModel;

};

#endif // COMPONENTBROWSER_H
