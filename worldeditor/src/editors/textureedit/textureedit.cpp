#include "textureedit.h"
#include "ui_textureedit.h"

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

#include "projectmanager.h"

#define SCALE 100.0f

TextureEdit::TextureEdit(DocumentModel *document) :
        QWidget(nullptr),
        ui(new Ui::TextureEdit),
        m_pRender(nullptr),
        m_pSettings(nullptr),
        m_pConverter(new TextureConverter),
        m_pDocument(document) {

    ui->setupUi(this);

    CameraCtrl *ctrl = new CameraCtrl(ui->preview);
    ctrl->blockRotations(true);
    ctrl->init(nullptr);
    ui->preview->setController(ctrl);
    ui->preview->setScene(Engine::objectCreate<Scene>("Scene"));

    connect(ui->preview, SIGNAL(inited()), this, SLOT(onGLInit()));
    startTimer(16);
}

TextureEdit::~TextureEdit() {
    delete ui;
}

void TextureEdit::timerEvent(QTimerEvent *) {
    ui->preview->repaint();
}

void TextureEdit::closeEvent(QCloseEvent *event) {
    if(!m_pDocument->checkSave(this)) {
        event->ignore();
        return;
    }
    QDir dir(ProjectManager::instance()->contentPath());
    m_pDocument->closeFile(dir.relativeFilePath(m_Path));
}

bool TextureEdit::isModified() const {
    return m_pSettings->isModified();
}

void TextureEdit::loadAsset(IConverterSettings *settings) {
    show();
    raise();

    if(m_pSettings) {
        disconnect(m_pSettings, &IConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
    }

    m_Path = settings->source();

    Resource *resource = Engine::loadResource<Resource>(qPrintable(settings->destination()));
    Sprite *sprite = dynamic_cast<Sprite *>(resource);
    if(sprite) {
        m_pRender->setSprite(sprite);
    } else {
        Texture *texture = dynamic_cast<Texture *>(resource);
        if(texture) {
            m_pRender->setTexture(texture);
        }
    }

    Camera *camera = ui->preview->controller()->camera();
    if(camera) {
        camera->actor()->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));
        camera->setOrthoSize(SCALE);
        camera->setFocal(SCALE);
    }

    m_pSettings = settings;
    connect(m_pSettings, &IConverterSettings::updated, this, &TextureEdit::onUpdateTemplate);
}

QStringList TextureEdit::assetTypes() const {
    return {"Texture", "Sprite"};
}

void TextureEdit::onUpdateTemplate() {
    TextureImportSettings *s = dynamic_cast<TextureImportSettings *>(m_pSettings);
    if(s) {
        m_pConverter->convertTexture(s, m_pRender->texture());
    }
}

void TextureEdit::onGLInit() {
    Scene *scene = ui->preview->scene();
    Camera *camera = ui->preview->controller()->camera();
    if(camera) {
        camera->setOrthographic(true);
    }

    Actor *object = Engine::objectCreate<Actor>("Sprite", scene);
    object->transform()->setScale(Vector3(SCALE));
    m_pRender = static_cast<SpriteRender *>(object->addComponent("SpriteRender"));
    if(m_pRender) {
        m_pRender->setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
    }
}
