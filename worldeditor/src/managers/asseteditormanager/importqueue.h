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
    explicit ImportQueue    (QWidget *parent = nullptr);
    ~ImportQueue            ();

    void                    init                (IconRender *render);

signals:
    void                    finished            ();

    void                    rendered            (const QString &uuid);

private slots:
    void                    onProcessed         (const QString &path, uint32_t type);

    void                    onStarted           (int count, const QString &action);
    void                    onFinished          ();

private:
    void                    keyPressEvent       (QKeyEvent *e);

    void                    renderIcons         ();

    Ui::ImportQueue        *ui;
    IconRender             *m_pIconRender;

    QMap<QString, uint8_t>  m_UpdateQueue;
};

#endif // IMPORTQUEUE_H
