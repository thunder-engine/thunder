#include "spriteelement.h"
#include "ui_spriteelement.h"

#include "../converter/textureconverter.h"

SpriteElement::SpriteElement(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SpriteElement),
        m_settings(nullptr) {

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
    m_settings = settings;
    connect(m_settings, &TextureImportSettings::updated, this, &SpriteElement::onElementUpdated);
}

void SpriteElement::onSelectionChanged(const QString &key) {
    m_key = key;
    onElementUpdated();
}

void SpriteElement::onElementUpdated() {
    if(m_settings) {
        auto it = m_settings->elements().find(m_key.toStdString());
        if(it != m_settings->elements().end()) {
            TextureImportSettings::Element element = it->second;

            ui->nameEdit->blockSignals(true);
            ui->nameEdit->setText(m_key);
            ui->nameEdit->blockSignals(false);

            ui->xEdit->blockSignals(true);
            ui->xEdit->setText(QString::number((int)element.m_min.x));
            ui->xEdit->blockSignals(false);

            ui->yEdit->blockSignals(true);
            ui->yEdit->setText(QString::number((int)element.m_max.y));
            ui->yEdit->blockSignals(false);

            ui->wEdit->blockSignals(true);
            ui->wEdit->setText(QString::number(int(element.m_max.x - element.m_min.x)));
            ui->wEdit->blockSignals(false);

            ui->hEdit->blockSignals(true);
            ui->hEdit->setText(QString::number(int(element.m_max.y - element.m_min.y)));
            ui->hEdit->blockSignals(false);

            ui->borderEditL->blockSignals(true);
            ui->borderEditL->setText(QString::number(element.m_borderMin.x));
            ui->borderEditL->blockSignals(false);

            ui->borderEditR->blockSignals(true);
            ui->borderEditR->setText(QString::number(element.m_borderMax.x));
            ui->borderEditR->blockSignals(false);

            ui->borderEditT->blockSignals(true);
            ui->borderEditT->setText(QString::number(element.m_borderMax.y));
            ui->borderEditT->blockSignals(false);

            ui->borderEditB->blockSignals(true);
            ui->borderEditB->setText(QString::number(element.m_borderMin.y));
            ui->borderEditB->blockSignals(false);

            Vector2 pivot = element.m_pivot;
            ui->pivotXEdit->blockSignals(true);
            ui->pivotXEdit->setText(QString::number(pivot.x, 'f', 4));
            ui->pivotXEdit->blockSignals(false);

            ui->pivotYEdit->blockSignals(true);
            ui->pivotYEdit->setText(QString::number(pivot.y, 'f', 4));
            ui->pivotYEdit->blockSignals(false);
        }
    }
}

void SpriteElement::onElementChanged() {
    if(m_settings) {
        TextureImportSettings::Element element;

        element.m_min.x = ui->xEdit->text().toInt();
        element.m_max.y = ui->yEdit->text().toInt();
        element.m_max.x = element.m_min.x + ui->wEdit->text().toInt();
        element.m_min.y = element.m_max.y - ui->hEdit->text().toInt();

        element.m_borderMin.x = ui->borderEditL->text().toUInt();
        element.m_borderMax.x = ui->borderEditR->text().toUInt();
        element.m_borderMax.y = ui->borderEditT->text().toUInt();
        element.m_borderMin.y = ui->borderEditB->text().toUInt();

        element.m_pivot = Vector2(ui->pivotXEdit->text().toFloat(),
                                  ui->pivotYEdit->text().toFloat());

        QString name = ui->nameEdit->text();
        if(name != m_key) {
            m_settings->removeElement(m_key.toStdString());
            m_key = name;
        }
        m_settings->setElement(element, m_key.toStdString());
    }
}

void SpriteElement::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
