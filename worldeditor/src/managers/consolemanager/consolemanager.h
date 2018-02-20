#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
    class ConsoleManager;
}

class LogModel;
class QMenu;

class ConsoleManager : public QWidget {
    Q_OBJECT
public:
    explicit ConsoleManager     (QWidget *parent = 0);
    ~ConsoleManager             ();

public slots:
    void                        onLogRecord                 (uint8_t type, const QString &str);

private slots:
    void                        on_clearButton_clicked      ();

    void                        on_consoleOutput_customContextMenuRequested (const QPoint &pos);

    void                        onCopy                      ();

private:
    Ui::ConsoleManager         *ui;

    LogModel                   *m_pItems;

    QMenu                      *m_pMenu;
};

#endif // CONSOLEMANAGER_H
