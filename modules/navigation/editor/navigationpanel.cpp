#include "navigationpanel.h"
#include "ui_navigationpanel.h"

#include <editor.h>
#include <editor/projectsettings.h>

#include <QSignalBlocker>

NavigationPanel::NavigationPanel(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::NavigationPanel) {
    ui->setupUi(this);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &NavigationPanel::onTypeSelected);
    connect(ui->addType, &QPushButton::clicked, this, &NavigationPanel::onAddType);
    connect(ui->removeType, &QPushButton::clicked, this, &NavigationPanel::onRemoveType);
    connect(ui->lineName, &QLineEdit::editingFinished, this, &NavigationPanel::updateAgentType);
    connect(ui->lineRadius, &QLineEdit::editingFinished, this, &NavigationPanel::updateAgentType);
    connect(ui->lineHeight, &QLineEdit::editingFinished, this, &NavigationPanel::updateAgentType);
    connect(ui->lineStep, &QLineEdit::editingFinished, this, &NavigationPanel::updateAgentType);
    connect(ui->sliderSlope, &QSlider::valueChanged, this, &NavigationPanel::updateAgentType);

    loadSettings();
}

NavigationPanel::~NavigationPanel() {
    delete ui;
}

void NavigationPanel::loadSettings() {
    VariantMap navigation = Editor::project()->property("navigation").toMap();
    auto it = navigation.find("navmesh");
    if(it != navigation.end()) {
        for(const Variant &item : it->second.toList()) {
            VariantMap data = item.toMap();
            if(data.empty()) {
                continue;
            }

            AgentType type;
            type.name = data["name"].toString();
            type.height = data["height"].toFloat();
            type.radius = data["radius"].toFloat();
            type.maxClimb = data["maxClimb"].toFloat();
            type.maxSlope = data["maxSlope"].toFloat();
            m_agentTypes.push_back(type);
        }
    }

    if(m_agentTypes.empty()) {
        m_agentTypes.push_back(AgentType());
    }

    for(size_t i = 0; i < m_agentTypes.size(); i++) {
        NavigationSystem::setAgentType(i, m_agentTypes[i]);
    }
    updateEditor();
}

void NavigationPanel::saveSettings() {
    VariantList types;
    for(const AgentType &type : m_agentTypes) {
        types.push_back(VariantMap({
            {"name", type.name},
            {"height", type.height},
            {"radius", type.radius},
            {"maxClimb", type.maxClimb},
            {"maxSlope", type.maxSlope}
        }));
    }

    VariantMap navigation = Editor::project()->property("navigation").toMap();
    navigation["navmesh"] = types;
    Editor::project()->setProperty("navigation", navigation);
    Editor::project()->saveSettings();

    for(size_t i = 0; i < m_agentTypes.size(); i++) {
        NavigationSystem::setAgentType(i, m_agentTypes[i]);
    }
}

void NavigationPanel::updateEditor() {
    m_updating = true;
    ui->listWidget->clear();
    for(const AgentType &type : m_agentTypes) {
        ui->listWidget->addItem(type.name.data());
    }
    if(!m_agentTypes.empty()) {
        ui->listWidget->setCurrentRow(0);
    }
    m_updating = false;
    if(!m_agentTypes.empty()) {
        onTypeSelected(0);
    }
}

void NavigationPanel::updateAgentType() {
    if(m_updating) {
        return;
    }

    int index = ui->listWidget->currentRow();
    if(index < 0 || index >= static_cast<int>(m_agentTypes.size())) {
        return;
    }

    AgentType &type = m_agentTypes[index];
    type.name = ui->lineName->text().toStdString();
    type.radius = ui->lineRadius->text().toFloat();
    type.height = ui->lineHeight->text().toFloat();
    type.maxClimb = ui->lineStep->text().toFloat();
    type.maxSlope = ui->sliderSlope->value();
    ui->listWidget->item(index)->setText(ui->lineName->text());
    saveSettings();
}

void NavigationPanel::onTypeSelected(int row) {
    if(m_updating || row < 0 || row >= static_cast<int>(m_agentTypes.size())) {
        return;
    }

    m_updating = true;
    const AgentType &type = m_agentTypes[row];
    ui->lineName->setText(type.name.data());
    ui->lineRadius->setText(QString::number(type.radius));
    ui->lineHeight->setText(QString::number(type.height));
    ui->lineStep->setText(QString::number(type.maxClimb));
    ui->sliderSlope->setValue(static_cast<int>(type.maxSlope));
    m_updating = false;
}

void NavigationPanel::onAddType() {
    AgentType type;
    type.name = TString("Agent ") + std::to_string(m_agentTypes.size());
    m_agentTypes.push_back(type);
    updateEditor();
    ui->listWidget->setCurrentRow(static_cast<int>(m_agentTypes.size()) - 1);
    saveSettings();
}

void NavigationPanel::onRemoveType() {
    if(m_agentTypes.size() <= 1) {
        return;
    }

    int index = ui->listWidget->currentRow();
    if(index < 0 || index >= static_cast<int>(m_agentTypes.size())) {
        return;
    }

    m_agentTypes.erase(m_agentTypes.begin() + index);
    updateEditor();
    saveSettings();
}
