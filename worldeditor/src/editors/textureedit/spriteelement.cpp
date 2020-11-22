#include "spriteelement.h"
#include "ui_spriteelement.h"

#include <QPainter>

#include "textureconverter.h"

SpriteElement::SpriteElement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpriteElement),
    m_pSettings(nullptr) {

    ui->setupUi(this);

    connect(ui->xEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->yEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->wEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->hEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));

    connect(ui->pivotXEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->pivotYEdit, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
}

SpriteElement::~SpriteElement() {
    delete ui;
}

void SpriteElement::setSettings(TextureImportSettings *settings) {
    m_pSettings = settings;
    connect(m_pSettings, &TextureImportSettings::updated, this, &SpriteElement::onElementUpdated);
}

void SpriteElement::onSelectionChanged(const QString &key) {
    m_Key = key;
    onElementUpdated();
}

void SpriteElement::onElementUpdated() {
    if(m_pSettings) {
        QRect rect = m_pSettings->elements().value(m_Key).m_Rect;
        ui->xEdit->blockSignals(true);
        ui->xEdit->setText(QString::number(rect.x()));
        ui->xEdit->blockSignals(false);

        ui->yEdit->blockSignals(true);
        ui->yEdit->setText(QString::number(rect.y()));
        ui->yEdit->blockSignals(false);

        ui->wEdit->blockSignals(true);
        ui->wEdit->setText(QString::number(rect.width()));
        ui->wEdit->blockSignals(false);

        ui->hEdit->blockSignals(true);
        ui->hEdit->setText(QString::number(rect.height()));
        ui->hEdit->blockSignals(false);

        Vector2 pivot = m_pSettings->elements().value(m_Key).m_Pivot;
        ui->pivotXEdit->blockSignals(true);
        ui->pivotXEdit->setText(QString::number(pivot.x));
        ui->pivotXEdit->blockSignals(false);

        ui->pivotYEdit->blockSignals(true);
        ui->pivotYEdit->setText(QString::number(pivot.y));
        ui->pivotYEdit->blockSignals(false);
    }
}

void SpriteElement::onElementChanged() {
    if(m_pSettings) {
        TextureImportSettings::Element element;

        element.m_Rect.setX(ui->xEdit->text().toInt());
        element.m_Rect.setY(ui->yEdit->text().toInt());
        element.m_Rect.setWidth(ui->wEdit->text().toInt());
        element.m_Rect.setHeight(ui->hEdit->text().toInt());

        element.m_Pivot = Vector2(ui->pivotXEdit->text().toFloat(),
                                  ui->pivotYEdit->text().toFloat());

        m_pSettings->setElement(element, m_Key);
    }
}

void SpriteElement::paintEvent(QPaintEvent *pe) {
    QWidget::paintEvent(pe);

    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(66, 66, 66));
    painter.drawRoundedRect(rect(), 4, 4);

    painter.end();
}
