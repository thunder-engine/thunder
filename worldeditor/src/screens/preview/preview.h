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

    void setEngine(Engine *engine);

    bool isGamePause() const;
    void setGamePause(bool pause);

private slots:
    void onDraw();

private:
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    Ui::Preview *ui;

    QWindow *m_rhiWindow;

    Engine *m_engine;

    QPoint m_savedMousePos;

    bool m_gamePause;

};

#endif // PREVIEW_H
