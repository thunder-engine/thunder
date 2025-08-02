#ifndef IMPORTQUEUE_H
#define IMPORTQUEUE_H

#include <QDialog>

#include <set>

#include <stdint.h>
#include <astring.h>

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
    void onProcessed(const TString &path);

    void onStarted(int count, const TString &action);
    void onImportFinished();

private:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::ImportQueue *ui;

    std::set<TString> m_iconQueue;

    IconRender *m_render;
};

#endif // IMPORTQUEUE_H
