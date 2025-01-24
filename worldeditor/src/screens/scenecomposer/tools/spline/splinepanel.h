#ifndef SPLINEPANEL_H
#define SPLINEPANEL_H

#include <QWidget>

class SplineTool;

namespace Ui {
    class SplinePanel;
}

class SplinePanel : public QWidget {
    Q_OBJECT

public:
    explicit SplinePanel(QWidget *parent = nullptr);
    ~SplinePanel();

    void setTool(SplineTool *tool);

    void update();

private slots:
    void onEditFinished();

    void on_breakPoint_clicked();

    void on_addPoint_clicked();

    void on_deletePoint_clicked();

private:
    Ui::SplinePanel *ui;

    SplineTool *m_tool;

};

#endif // SPLINEPANEL_H
