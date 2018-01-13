#include "Actions.h"
#include "ui_Actions.h"

#include <QMenu>

Actions::Actions(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Actions) {

    ui->setupUi(this);

    setMenu(nullptr);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setMenu(QMenu *menu) {
    if(menu) {
        ui->toolButton->setMenu(menu);
        ui->toolButton->show();
    } else {
        ui->toolButton->hide();
    }
}
