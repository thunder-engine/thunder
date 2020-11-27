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

    connect(ui->borderEditL, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->borderEditR, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->borderEditT, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));
    connect(ui->borderEditB, SIGNAL(textEdited(QString)), this, SLOT(onElementChanged()));

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

        QRect border = m_pSettings->elements().value(m_Key).m_Border;
        ui->borderEditL->blockSignals(true);
        ui->borderEditL->setText(QString::number(border.left()));
        ui->borderEditL->blockSignals(false);

        ui->borderEditR->blockSignals(true);
        ui->borderEditR->setText(QString::number(border.right()));
        ui->borderEditR->blockSignals(false);

        ui->borderEditT->blockSignals(true);
        ui->borderEditT->setText(QString::number(border.top()));
        ui->borderEditT->blockSignals(false);

        ui->borderEditB->blockSignals(true);
        ui->borderEditB->setText(QString::number(border.bottom()));
        ui->borderEditB->blockSignals(false);

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

        element.m_Rect.setX      (ui->xEdit->text().toInt());
        element.m_Rect.setY      (ui->yEdit->text().toInt());
        element.m_Rect.setWidth  (ui->wEdit->text().toInt());
        element.m_Rect.setHeight (ui->hEdit->text().toInt());

        element.m_Border.setLeft  (ui->borderEditL->text().toUInt());
        element.m_Border.setRight (ui->borderEditR->text().toUInt());
        element.m_Border.setTop   (ui->borderEditT->text().toUInt());
        element.m_Border.setBottom(ui->borderEditB->text().toUInt());

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
