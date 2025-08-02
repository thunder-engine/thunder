#ifndef EDITORSETTINGSBROWSER_H
#define EDITORSETTINGSBROWSER_H

#include <QWidget>

#include <engine.h>

namespace Ui {
    class EditorSettingsBrowser;
}

class QAbstractItemModel;

class EditorSettingsBrowser : public QWidget {
    Q_OBJECT
public:
    explicit EditorSettingsBrowser(QWidget *parent = 0);
    ~EditorSettingsBrowser();

    void init();

private slots:
    void onSettingsUpdated(const Object::ObjectList &objects, const TString &property, Variant value);

    void on_groups_clicked(const QModelIndex &index);

private:
    void changeEvent(QEvent *event) override;

    Ui::EditorSettingsBrowser *ui;

    QAbstractItemModel *m_groupModel;

};

#endif // EDITORSETTINGSBROWSER_H
