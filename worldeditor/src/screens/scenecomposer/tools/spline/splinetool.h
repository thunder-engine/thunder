#ifndef SPLINETOOL_H
#define SPLINETOOL_H

#include "editor/editortool.h"

#include <components/spline.h>

#include "splinepanel.h"
#include "../../objectcontroller.h"

class Spline;

class SplineTool : public EditorTool {
public:
    explicit SplineTool(ObjectController *controller);

    ObjectController *controller() { return m_controller; }

    int point() const { return m_point; }
    void setPoint(int point);

    int tangent() const { return m_tangent; }
    void setTangent(int tangent) { m_tangent = tangent; }

    void update();

    Spline *spline();

private:
    void update(bool center, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

    bool blockSelection() const override;

    QWidget *panel() override;

private:
    Vector4 m_dotColor;
    Vector4 m_dotColorSelected;
    Vector4 m_lineColor;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;
    Vector3 m_positionIn;
    Vector3 m_positionOut;

    ObjectController *m_controller;

    Spline *m_spline;

    SplinePanel *m_splinePanel;

    float m_dotSize;

    int m_point;
    int m_tangent;

    bool m_canceled;

};

class SelectSplinePoint : public UndoCommand {
public:
    SelectSplinePoint(int point, int tangent, SplineTool *tool, const TString &name = QObject::tr("Select Spline Point").toStdString(), UndoCommand *group = nullptr);
    void undo() override { redo(); }
    void redo() override;

protected:
    SplineTool *m_tool;

    int m_point;
    int m_tangent;

};

class ChangeSplinePoint : public UndoCommand {
public:
    ChangeSplinePoint(const Spline::Point &point, SplineTool *tool, const TString &name = QObject::tr("Change Spline Point").toStdString(), UndoCommand *group = nullptr);
    void undo() override { redo(); }
    void redo() override;

protected:
    SplineTool *m_tool;

    Spline::Point m_point;

};

class DeleteSplinePoint : public UndoCommand {
public:
    DeleteSplinePoint(SplineTool *tool, const TString &name = QObject::tr("Delete Spline Point").toStdString(), UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    SplineTool *m_tool;

    Spline::Point m_point;

    int m_index;
    int m_tangent;

};

class InsertSplinePoint : public UndoCommand {
public:
    InsertSplinePoint(float factor, SplineTool *tool, const TString &name = QObject::tr("Duplicate Spline Point").toStdString(), UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    SplineTool *m_tool;

    float m_factor;

    int m_index;

};

#endif // SPLINETOOL_H
