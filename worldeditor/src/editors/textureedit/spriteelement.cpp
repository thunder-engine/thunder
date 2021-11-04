#include "spriteelement.h"
#include "ui_spriteelement.h"

#include <QPainter>

#include "textureconverter.h"

SpriteElement::SpriteElement(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SpriteElement),
        m_pSettings(nullptr) {

    ui->setupUi(this);

    connect(ui->nameEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));

    connect(ui->xEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->yEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->wEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->hEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));

    connect(ui->borderEditL, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->borderEditR, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->borderEditT, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->borderEditB, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));

    connect(ui->pivotXEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
    connect(ui->pivotYEdit, SIGNAL(editingFinished()), this, SLOT(onElementChanged()));
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

        ui->nameEdit->blockSignals(true);
        ui->nameEdit->setText(m_Key);
        ui->nameEdit->blockSignals(false);

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

        ui->borderEditL->blockSignals(true);
        ui->borderEditL->setText(QString::number(m_pSettings->elements().value(m_Key).m_BorderL));
        ui->borderEditL->blockSignals(false);

        ui->borderEditR->blockSignals(true);
        ui->borderEditR->setText(QString::number(m_pSettings->elements().value(m_Key).m_BorderR));
        ui->borderEditR->blockSignals(false);

        ui->borderEditT->blockSignals(true);
        ui->borderEditT->setText(QString::number(m_pSettings->elements().value(m_Key).m_BorderT));
        ui->borderEditT->blockSignals(false);

        ui->borderEditB->blockSignals(true);
        ui->borderEditB->setText(QString::number(m_pSettings->elements().value(m_Key).m_BorderB));
        ui->borderEditB->blockSignals(false);

        Vector2 pivot = m_pSettings->elements().value(m_Key).m_Pivot;
        ui->pivotXEdit->blockSignals(true);
        ui->pivotXEdit->setText(QString::number(pivot.x, 'f', 4));
        ui->pivotXEdit->blockSignals(false);

        ui->pivotYEdit->blockSignals(true);
        ui->pivotYEdit->setText(QString::number(pivot.y, 'f', 4));
        ui->pivotYEdit->blockSignals(false);
    }
}

void SpriteElement::onElementChanged() {
    if(m_pSettings) {
        TextureImportSettings::Element element;

        element.m_Rect.setX     (ui->xEdit->text().toInt());
        element.m_Rect.setY     (ui->yEdit->text().toInt());
        element.m_Rect.setWidth (ui->wEdit->text().toInt());
        element.m_Rect.setHeight(ui->hEdit->text().toInt());

        element.m_BorderL = ui->borderEditL->text().toUInt();
        element.m_BorderR = ui->borderEditR->text().toUInt();
        element.m_BorderT = ui->borderEditT->text().toUInt();
        element.m_BorderB = ui->borderEditB->text().toUInt();

        element.m_Pivot = Vector2(ui->pivotXEdit->text().toFloat(),
                                  ui->pivotYEdit->text().toFloat());

        QString name = ui->nameEdit->text();
        if(name != m_Key) {
            m_pSettings->removeElement(m_Key);
            m_Key = name;
        }
        m_pSettings->setElement(element, m_Key);
    }
}

void SpriteElement::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
