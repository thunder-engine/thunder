#include "preview.h"
#include "ui_preview.h"

#include <engine.h>

Preview::Preview(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Preview) {

    ui->setupUi(this);

    ui->viewport->setWorld(Engine::world());
    ui->viewport->setGameView(true);
    ui->viewport->init(); // must be called after all options set
}

bool Preview::isGamePause() const {
    return ui->viewport->isGamePaused();
}

void Preview::setGamePause(bool pause) {
    ui->viewport->setGamePaused(pause);
}
