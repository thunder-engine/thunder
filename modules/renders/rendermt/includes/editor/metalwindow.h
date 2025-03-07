#ifndef METALWINDOW_H
#define METALWINDOW_H

#include "rendermtsystem.h"

#include <QWindow>


class ViewDelegate;

class ThunderMetalWindow : public QWindow {
    Q_OBJECT

public:
    explicit ThunderMetalWindow(RenderMtSystem *system);

signals:
    void draw();

private:
    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;

private:
    RenderMtSystem *m_system;

    ViewDelegate *m_delegate;
};

#endif // METALWINDOW_H
