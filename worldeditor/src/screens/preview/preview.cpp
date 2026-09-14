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
#include "preview.h"
#include "ui_preview.h"

#include <engine.h>
#include <timer.h>
#include <world.h>
#include <camera.h>

#include <QMenu>
#include <QWindow>

Preview::Preview(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Preview) {

    ui->setupUi(this);

    ui->viewport->setWorld(Engine::world());
    ui->viewport->setGameView(true);
    ui->viewport->init(); // must be called after all options set

    ui->renderMode->setMenu(new QMenu);
    ui->viewport->createMenu(ui->renderMode->menu());
}

void Preview::onActivate() {
    Timer::reset();
    ui->viewport->rhiWindow()->requestActivate();
}

bool Preview::isPaused() const {
    return ui->viewport->isGamePaused();
}

void Preview::setPaused(bool pause) {
    ui->viewport->setGamePaused(pause);
}
