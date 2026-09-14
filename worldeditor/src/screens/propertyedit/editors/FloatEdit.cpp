/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "FloatEdit.h"
#include "ui_FloatEdit.h"

#include <float.h>

#include <QDoubleValidator>
#include <QTimer>

#define SCALE 100

FloatEdit::FloatEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::FloatEdit) {
    ui->setupUi(this);

    QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    ui->lineEdit->setValidator(validator);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &FloatEdit::editFinished);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &FloatEdit::onValueChanged);

    ui->lineEdit->installEventFilter(this);

    ui->horizontalSlider->setVisible(false);
}

FloatEdit::~FloatEdit() {
    delete ui;
}

void FloatEdit::setEditorHint(const TString &hint) {
    if(!hint.isEmpty()) {
        static QRegularExpression regExp {"\\d+\\.\\d+"};

        QStringList list;

        auto it = regExp.globalMatch(hint.data());
        while(it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            list << match.captured(0).trimmed();
        }

        if(list.size() == 2) {
            ui->horizontalSlider->setRange(list[0].toFloat() * SCALE, list[1].toFloat() * SCALE);

            ui->horizontalSlider->setVisible(true);
        }
    }
}

Variant FloatEdit::data() const {
    return ui->lineEdit->text().toFloat();
}

void FloatEdit::setData(const Variant &value) {
    static const QRegularExpression reg("\\.?0+$");
    ui->lineEdit->setText(QString::number(value.toFloat(), 'f', 4).remove(reg));
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(value.toFloat() * SCALE);
    ui->horizontalSlider->blockSignals(false);
}

void FloatEdit::onValueChanged(int value) {
    ui->lineEdit->setText(QString::number((double)value / (double)SCALE, 'f', 4));

    emit editFinished();
}

bool FloatEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
