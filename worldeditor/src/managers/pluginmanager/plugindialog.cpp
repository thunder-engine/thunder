#include <QFileDialog>

#include "plugindialog.h"

#include "ui_plugindialog.h"

#include "pluginmodel.h"
#include "plugindelegate.h"

#include "codemanager.h"

PluginDialog::PluginDialog(Engine *engine, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::PluginDialog) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    PluginModel *model  = PluginModel::instance();
    connect(model, SIGNAL(updated()), this, SIGNAL(updated()));
    connect(model, SIGNAL(pluginReloaded(QString)), this, SIGNAL(pluginReloaded()));
    connect(CodeManager::instance(), SIGNAL(buildSucess(QString)), model, SLOT(reloadPlugin(QString)));

    model->init(engine);
    model->rescan();

    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new PluginDelegate(this));
    ui->tableView->horizontalHeader()->setHighlightSections(false);
}

PluginDialog::~PluginDialog() {
    delete ui;
}

void PluginDialog::on_closeButton_clicked() {
    accept();
}

void PluginDialog::on_loadButton_clicked() {
    QDir dir        = QDir(QDir::currentPath());
    QString path    = QFileDialog::getOpenFileName(this,
                                                    tr("Please select Thunder Engine Mod"),
                                                    dir.absolutePath(),
                                                    tr("Mods (*.dll *.mod)") );
    if( !path.isEmpty() ) {
        PluginModel *model  = PluginModel::instance();
        if( model->loadPlugin(dir.relativeFilePath(path)) ) {
            ui->tableView->setModel(model);
            emit updated();
        }
    }
}
