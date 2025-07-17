#include "shadercodedialog.h"
#include "ui_shadercodedialog.h"

#include <pluginmanager.h>

#include <QPlainTextEdit>

#include <editor/asseteditor.h>

#include "../converter/shaderbuilder.h"

ShaderCodeDialog::ShaderCodeDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ShaderCodeDialog) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showShader()));

    m_codeEditor = reinterpret_cast<AssetEditor *>(PluginManager::instance()->getPluginObject("CodeEdit"));
    if(m_codeEditor) {
        ui->verticalLayout->addWidget(m_codeEditor);
    } else {
        m_plainText = new QPlainTextEdit;
        m_plainText->setReadOnly(true);
        ui->verticalLayout->addWidget(m_plainText);
    }
}

ShaderCodeDialog::~ShaderCodeDialog() {
    delete ui;
}

void ShaderCodeDialog::setData(const VariantMap &data) {
    m_data = data;

    showShader();
}

void ShaderCodeDialog::showShader() {
    auto it = m_data.find(ui->comboBox->currentIndex() == 0 ? FRAGMENT : STATIC);
    if(it == m_data.end()) {
        return;
    }

    TString text = it->second.toString();
    if(m_codeEditor) {
        m_codeEditor->loadData(text, "glsl");
    } else {
        m_plainText->setPlainText(text.data());
    }
}
