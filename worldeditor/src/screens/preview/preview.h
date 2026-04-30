#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>

namespace Ui {
    class Preview;
}

class Preview : public QWidget {
    Q_OBJECT
public:
    Preview(QWidget *parent = nullptr);

    void onActivate();

    bool isPaused() const;
    void setPaused(bool pause);

protected:
    Ui::Preview *ui;

};

#endif // PREVIEW_H
