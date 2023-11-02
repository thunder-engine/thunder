#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>

class Engine;

namespace Ui {
    class Preview;
}

class Preview : public QWidget {
    Q_OBJECT
public:
    Preview(QWidget *parent = nullptr);

    bool isGamePause() const;
    void setGamePause(bool pause);

protected:
    Ui::Preview *ui;

};

#endif // PREVIEW_H
