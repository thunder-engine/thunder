#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
    class ConsoleManager;
}

class LogModel;
class QMenu;
class QLog;

class ConsoleManager : public QWidget {
    Q_OBJECT
public:
    explicit ConsoleManager(QWidget *parent = 0);
    ~ConsoleManager();

public slots:
    void onLogRecord(uint8_t type, const QString &str);

    void parseLogs(const QString &log);

private slots:
    void on_clearButton_clicked();

    void on_consoleOutput_customContextMenuRequested(const QPoint &pos);

    void onCopy();

private:
    void changeEvent(QEvent *event) override;

    Ui::ConsoleManager *ui;

    LogModel *m_model;

    QMenu *m_menu;

    QLog *m_handler;
};

#endif // CONSOLEMANAGER_H
