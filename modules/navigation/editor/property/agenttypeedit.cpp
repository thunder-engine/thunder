#include "agenttypeedit.h"
#include "ui_agenttypeedit.h"

#include <navigationsystem.h>

AgentTypeEdit::AgentTypeEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::AgentTypeEdit) {
    ui->setupUi(this);

    for(int i = 0; i < NavigationSystem::agentTypeCount(); ++i) {
        AgentType type = NavigationSystem::agentType(i);
        ui->comboBox->addItem(type.name.data());
    }

    connect(ui->comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        emit dataChanged();
        emit editFinished();
    });
}

AgentTypeEdit::~AgentTypeEdit() {
    delete ui;
}

Variant AgentTypeEdit::data() const {
    return ui->comboBox->currentIndex();
}

void AgentTypeEdit::setData(const Variant &data) {
    int index = data.toInt();
    if(index >= 0 && index < ui->comboBox->count()) {
        ui->comboBox->setCurrentIndex(index);
    }
}
