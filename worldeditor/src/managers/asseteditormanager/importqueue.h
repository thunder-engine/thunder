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
class Engine;
class QOpenGLContext;

class ImportQueue : public QDialog {
    Q_OBJECT

public:
    explicit ImportQueue(Engine *engine, QWidget *parent = nullptr);
    ~ImportQueue();

    void init(IconRender *render);

signals:
    void finished();

    void rendered(const QString &uuid);

private slots:
    void onProcessed(const QString &path, const QString &type);

    void onStarted(int count, const QString &action);
    void onImportFinished();

private:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *event) override;


    Ui::ImportQueue        *ui;

    QMap<QString, QString> m_UpdateQueue;

    Engine                 *m_pEngine;

    IconRender             *m_pRender;
};

#endif // IMPORTQUEUE_H
