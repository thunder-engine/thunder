#include "textureedit.h"
#include "ui_textureedit.h"

#include <QMessageBox>

#include "textureconverter.h"

#include "editors/propertyedit/nextobject.h"
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/spriterender.h"
#include "components/camera.h"

#include "resources/texture.h"
#include "resources/material.h"

#define SCALE 100.0f

TextureEdit::TextureEdit() :
        QMainWindow(nullptr),
        m_Modified(false),
        ui(new Ui::TextureEdit) {

    ui->setupUi(this);

    CameraCtrl *ctrl  = new CameraCtrl(ui->Preview);
    ctrl->blockRotations(true);
    ctrl->init(nullptr);
    ui->Preview->setController(ctrl);
    ui->Preview->setScene(Engine::objectCreate<Scene>("Scene"));
    ui->Preview->setWindowTitle("Preview");

    ui->treeView->setWindowTitle("Properties");

    connect(ui->Preview, SIGNAL(inited()), this, SLOT(onGLInit()));
    startTimer(16);

    ui->centralwidget->addToolWindow(ui->Preview, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->treeView, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(ui->Preview));

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = ui->menuWindow->addAction(it->windowTitle());
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));

    readSettings();

    m_pConverter = new TextureConverter;
}

TextureEdit::~TextureEdit() {
    delete ui;
}

void TextureEdit::timerEvent(QTimerEvent *) {
    ui->Preview->repaint();
}

void TextureEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("texture.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("texture.windows"));
}

void TextureEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("texture.geometry", saveGeometry());
    settings.setValue("texture.windows", ui->centralwidget->saveState());
}

void TextureEdit::closeEvent(QCloseEvent *event) {
    writeSettings();

    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The %1 import settings has been modified.").arg(tr("texture")));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if(result == QMessageBox::Yes) {
            on_actionSave_triggered();
        }
    }
}

bool TextureEdit::isModified() const {
    return m_Modified;
}

void TextureEdit::loadAsset(IConverterSettings *settings) {
    show();
    raise();
    m_Modified = false;

    if(m_pSprite) {
        m_pTexture = Engine::loadResource<Texture>(settings->destination());
        m_pSprite->setTexture(m_pTexture);
    }

    Camera *camera = ui->Preview->controller()->camera();
    if(camera) {
        camera->actor()->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));
        camera->setOrthoSize(SCALE);
        camera->setFocal(SCALE);
    }

    m_pSettings = settings;
    connect(m_pSettings, SIGNAL(updated()), this, SLOT(onUpdateTemplate()));
    ui->treeView->setObject(m_pSettings);
}

void TextureEdit::onUpdateTemplate(bool update) {
    m_Modified = update;

    m_pTexture->loadUserData(m_pConverter->convertResource(m_pSettings));
}

void TextureEdit::onGLInit() {
    Scene *scene    = ui->Preview->scene();
    Camera *camera  = ui->Preview->controller()->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object   = Engine::objectCreate<Actor>("Sprite", scene);
    object->transform()->setScale(Vector3(SCALE));
    m_pSprite       = static_cast<SpriteRender *>(object->addComponent("SpriteRender"));
    if(m_pSprite) {
        m_pSprite->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
    }
}

void TextureEdit::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void TextureEdit::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}

void TextureEdit::on_actionSave_triggered() {
    m_pSettings->saveSettings();
    m_Modified = false;
}
