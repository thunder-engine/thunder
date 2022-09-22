#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QWidget>

class Engine;

class SceneView : public QWidget {
    Q_OBJECT
public:
    SceneView(QWidget *parent = nullptr);
    ~SceneView() override;

    void setEngine(Engine *engine);

    bool isGamePause() const;
    void setGamePause(bool pause);

private slots:
    void onDraw();

private:
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    QWindow *m_rhiWindow;

    Engine *m_engine;

    bool m_gamePause;

};

#endif // SCENEVIEW_H
