#ifndef IMPORTQUEUE_H
#define IMPORTQUEUE_H

#include <QDialog>
#include <QMap>

#include <stdint.h>

namespace Ui {
    class ImportQueue;
}

class BaseConverterSettings;
class IconRender;

class ImportQueue : public QDialog {
    Q_OBJECT

public:
    explicit ImportQueue(QWidget *parent = nullptr);
    ~ImportQueue();

    void init(IconRender *render);

signals:
    void importFinished();

private slots:
    void onProcessed(const QString &path, const QString &type);

    void onStarted(int count, const QString &action);
    void onImportFinished();

private:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::ImportQueue *ui;

    QMap<QString, QString> m_updateQueue;

    IconRender *m_render;
};

#endif // IMPORTQUEUE_H
