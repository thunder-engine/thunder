#include "spriteelement.h"
#include "ui_spriteelement.h"

#include <QPainter>

#include "textureconverter.h"

SpriteElement::SpriteElement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpriteElement),
    m_pSettings(nullptr) {

    ui->setupUi(this);

    connect(ui->xSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onElementChanged()));
    connect(ui->ySpinBox, SIGNAL(valueChanged(int)), this, SLOT(onElementChanged()));
    connect(ui->wSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onElementChanged()));
    connect(ui->hSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onElementChanged()));
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
        QRect rect = m_pSettings->elements()[m_Key];
        ui->xSpinBox->blockSignals(true);
        ui->xSpinBox->setValue(rect.x());
        ui->xSpinBox->blockSignals(false);

        ui->ySpinBox->blockSignals(true);
        ui->ySpinBox->setValue(rect.y());
        ui->ySpinBox->blockSignals(false);

        ui->wSpinBox->blockSignals(true);
        ui->wSpinBox->setValue(rect.width());
        ui->wSpinBox->blockSignals(false);

        ui->hSpinBox->blockSignals(true);
        ui->hSpinBox->setValue(rect.height());
        ui->hSpinBox->blockSignals(false);
    }
}

void SpriteElement::onElementChanged() {
    if(m_pSettings) {
        QRect rect;
        rect.setX(ui->xSpinBox->value());
        rect.setY(ui->ySpinBox->value());
        rect.setWidth(ui->wSpinBox->value());
        rect.setHeight(ui->hSpinBox->value());

        m_pSettings->setElement(rect, m_Key);
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
