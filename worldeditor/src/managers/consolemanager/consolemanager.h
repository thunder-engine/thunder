#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
    class ConsoleManager;
}

class LogModel;

class ConsoleManager : public QWidget {
    Q_OBJECT
public:
    explicit ConsoleManager     (QWidget *parent = 0);
    ~ConsoleManager             ();

public slots:
    void                        onLogRecord                 (uint8_t type, const QString &str);

private slots:
    void                        on_clearButton_clicked      ();

    void                        on_infoButton_clicked       ();

    void                        on_warningButton_clicked    ();

    void                        on_errorButton_clicked      ();

private:
    Ui::ConsoleManager         *ui;

    LogModel                   *m_pItems;
};

#endif // CONSOLEMANAGER_H
