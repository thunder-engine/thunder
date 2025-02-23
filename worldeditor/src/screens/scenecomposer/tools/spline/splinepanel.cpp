#include "splinepanel.h"
#include "ui_splinepanel.h"

#include "splinetool.h"

#include <components/spline.h>

#include <float.h>
#include <QDoubleValidator>

SplinePanel::SplinePanel(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SplinePanel),
        m_tool(nullptr)  {

    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->xEdit->setValidator(validator);
    ui->yEdit->setValidator(validator);
    ui->zEdit->setValidator(validator);

    connect(ui->xEdit, &QLineEdit::editingFinished, this, &SplinePanel::onEditFinished);
    connect(ui->yEdit, &QLineEdit::editingFinished, this, &SplinePanel::onEditFinished);
    connect(ui->zEdit, &QLineEdit::editingFinished, this, &SplinePanel::onEditFinished);

    ui->breakPoint->setProperty("checkred", true);
}

SplinePanel::~SplinePanel() {
    delete ui;
}

void SplinePanel::setTool(SplineTool *tool) {
    m_tool = tool;
}

void SplinePanel::update() {
    int index = m_tool->point();
    Spline *spline = m_tool->spline();
    if(spline && index > -1) {
        Spline::Point p = spline->point(index);

        QRegularExpression reg("\\.?0+$");
        ui->xEdit->setText(QString::number(p.position.x, 'f', 4).remove(reg));
        ui->yEdit->setText(QString::number(p.position.y, 'f', 4).remove(reg));
        ui->zEdit->setText(QString::number(p.position.z, 'f', 4).remove(reg));

        ui->breakPoint->setChecked(p.breaked);
    }
}

void SplinePanel::onEditFinished() {
    int index = m_tool->point();
    Spline *spline = m_tool->spline();
    if(spline && index > -1) {
        Spline::Point p = spline->point(index);

        Vector3 v(ui->xEdit->text().toFloat(),
                  ui->yEdit->text().toFloat(),
                  ui->zEdit->text().toFloat());

        bool checked = ui->breakPoint->isChecked();
        if(p.position != v || p.breaked != checked) {
            Vector3 delta(v - p.position);
            p.position += delta;
            p.tangentIn += delta;
            p.tangentOut += delta;
            if(!checked && p.breaked != checked) {
                delta = p.tangentOut - p.position;
                p.tangentIn = p.position - delta;
            }
            p.breaked = checked;
            UndoManager::instance()->push(new ChangeSplinePoint(p, m_tool));
        }
    }
}

void SplinePanel::on_breakPoint_clicked() {
    onEditFinished();
}

void SplinePanel::on_addPoint_clicked() {
    UndoManager::instance()->push(new InsertSplinePoint(0.5f, m_tool));
}

void SplinePanel::on_deletePoint_clicked() {
    UndoManager::instance()->push(new DeleteSplinePoint(m_tool));
}
