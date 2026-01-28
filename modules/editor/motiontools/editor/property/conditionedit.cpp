#include "conditionedit.h"
#include "ui_conditionedit.h"

#include "statelink.h"

static const QRegularExpression reg("\\.?0+$");

ConditionEdit::ConditionEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ConditionEdit) {

    ui->setupUi(this);
    ui->lineEdit->setVisible(false);
}

ConditionEdit::~ConditionEdit() {
    delete ui;
}

void ConditionEdit::setObject(Object *object, const TString &property) {
    PropertyEdit::setObject(object, property);

    StateLink *link = dynamic_cast<StateLink *>(m_object);
    if(link) {
        m_graph = dynamic_cast<AnimationControllerGraph *>(link->sender->graph());

        if(m_graph) {
            ui->comboName->clear();
            ui->comboName->blockSignals(true);
            for(auto &it : m_graph->variables()) {
                ui->comboName->addItem(it.data());
            }
            ui->comboName->setCurrentIndex(-1);
            ui->comboName->blockSignals(false);
        }
    }
}

Variant ConditionEdit::data() const {
    return VariantMap({
        { gName, m_conditionName },
        { gType, m_conditionRule },
        { gValue, m_conditionValue }
    });
}

void ConditionEdit::setData(const Variant &data) {
    VariantMap map = data.toMap();
    if(map.size() >= 3) {
        ui->comboName->setCurrentText(map[gName].toString().data());
        int rule = map[gType].toInt();
        for(int i = 0; i < ui->comboRule->count(); i++) {
            if(ui->comboRule->itemData(i) == rule) {
                ui->comboRule->setCurrentIndex(i);
                break;
            }
        }

        m_conditionValue = map[gValue];
        if(m_conditionValue.type() == MetaType::BOOLEAN) {
            ui->checkBox->setChecked(m_conditionValue.toBool());
        } else {
            ui->lineEdit->setText(QString(m_conditionValue.toString().data()).remove(reg));
        }
    }
}

void ConditionEdit::on_comboName_currentIndexChanged(int index) {
    TString name(ui->comboName->itemText(index).toStdString());

    if(name != m_conditionName) {
        m_conditionName = name;
        if(m_graph) {
            m_conditionValue = m_graph->variable(m_conditionName);
        }
        ui->comboRule->clear();

        int32_t type = m_conditionValue.type();
        if(type == MetaType::BOOLEAN || type == MetaType::INTEGER) {
            ui->comboRule->addItem("Equals", AnimationTransitionCondition::Equals);
            ui->comboRule->addItem("Not Equals", AnimationTransitionCondition::NotEquals);
        }

        if(type == MetaType::INTEGER || type == MetaType::FLOAT) {
            ui->comboRule->addItem("Greater", AnimationTransitionCondition::Greater);
            ui->comboRule->addItem("Less", AnimationTransitionCondition::Less);
        }

        ui->comboRule->setCurrentIndex(0);

        bool isBool = (m_conditionValue.type() == MetaType::BOOLEAN);
        ui->checkBox->setVisible(isBool);
        ui->lineEdit->setVisible(!isBool);

        if(isBool) {
            ui->checkBox->setChecked(m_conditionValue.toBool());
        } else {
            if(m_conditionValue.type() == MetaType::INTEGER) {
                QIntValidator *validator = new QIntValidator(-INT32_MAX, INT32_MAX, this);
                ui->lineEdit->setValidator(validator);
            } else {
                QDoubleValidator *validator = new QDoubleValidator(-DBL_MAX, DBL_MAX, 4, this);
                validator->setLocale(QLocale("C"));
                ui->lineEdit->setValidator(validator);
            }

            ui->lineEdit->setText(QString(m_conditionValue.toString().data()).remove(reg));
        }

        emit editFinished();
    }
}


void ConditionEdit::on_comboRule_currentIndexChanged(int index) {
    int rule = ui->comboRule->itemData(index).toInt();

    if(rule != m_conditionRule) {
        m_conditionRule = rule;

        emit editFinished();
    }
}

void ConditionEdit::on_lineEdit_editingFinished() {
    TString text = ui->lineEdit->text().toStdString();
    if(m_conditionValue.toString() != text) {
        if(m_conditionValue.type() == MetaType::INTEGER) {
            m_conditionValue = text.toInt();
        } else {
            m_conditionValue = text.toFloat();
        }

        emit editFinished();
    }
}

void ConditionEdit::on_checkBox_toggled(bool checked) {
    if(m_conditionValue.toBool() != checked) {
        m_conditionValue = checked;

        emit editFinished();
    }
}
