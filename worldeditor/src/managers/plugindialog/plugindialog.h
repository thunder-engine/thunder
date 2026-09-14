/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>

class PluginManager;
class QSortFilterProxyModel;

namespace Ui {
    class PluginDialog;
}

class PluginDialog : public QDialog  {
    Q_OBJECT

public:
    PluginDialog(QWidget *parent = nullptr);
    ~PluginDialog();

public slots:
    void on_loadButton_clicked();

private slots:
    void on_restartButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

private:
    void changeEvent(QEvent *event) override;

    void updatePersistent(const QModelIndex &index);

protected:
    Ui::PluginDialog *ui;

    QSortFilterProxyModel *m_filter;

};

#endif // PLUGINDIALOG_H
