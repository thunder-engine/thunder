#include "configdialog.h"
#include "ui_configdialog.h"

#include "projectmanager.h"
#include "qbsbuilder.h"

#include <QFileDialog>
#include <QDebug>

#ifdef _WIN32
const QString gQBS("qbs.exe");
#else
const QString gQBS("qbs");
#endif

ConfigDialog::ConfigDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ConfigDialog) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    m_pBuilder  = new QbsBuilder();

    if(checkQbsVersion()) {

    }
}

ConfigDialog::~ConfigDialog() {
    delete ui;

    delete m_pBuilder;
}

bool ConfigDialog::checkQbsVersion() {
    QString version = m_pBuilder->builderVersion();
    if(!version.isEmpty()) {
        ui->version->setText(version);
        return true;
    }
    return false;
}

void ConfigDialog::on_pushOK_clicked() {
    accept();
}
