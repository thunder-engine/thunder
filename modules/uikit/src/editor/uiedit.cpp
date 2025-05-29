#include "uiedit.h"
#include "ui_uiedit.h"

#include <QFile>

#include <sstream>

#include <engine.h>

#include <world.h>
#include <scene.h>
#include <actor.h>
#include <camera.h>

#include <editor/undomanager.h>
#include <editor/assetconverter.h>
#include <editor/viewport/cameracontroller.h>

#include "components/uiloader.h"
#include "components/recttransform.h"
#include "utils/stringutil.h"

#include "widgetcontroller.h"

namespace {
    const char *gUiLoader("UiLoader");

    const char *gUi("ui");
    const char *gName("name");
    const char *gStyle("style");
    const char *gClass("class");

    const char *gCss("css=");
    const char *gEditorTag("editor=");
};

UiEdit::UiEdit() :
        ui(new Ui::UiEdit),
        m_world(Engine::objectCreate<World>("World")),
        m_scene(Engine::objectCreate<Scene>("Scene", m_world)),
        m_controller(nullptr),
        m_lastCommand(nullptr) {

    Actor *actor = Engine::composeActor(gUiLoader, "Screen", m_scene);
    m_loader = actor->getComponent<UiLoader>();

    ui->setupUi(this);

    m_controller = new WidgetController(m_loader);

    m_controller->doRotation(Vector3());
    m_controller->setGridAxis(CameraController::Axis::Z);
    m_controller->blockRotations(true);
    m_controller->setZoomLimits(Vector2(300, 1500));

    ui->preview->setController(m_controller);
    ui->preview->setWorld(m_world);
    ui->preview->init();
    ui->preview->setLiveUpdate(true);

    Camera *camera = m_controller->camera();
    if(camera) {
        camera->setScreenSpace(true);
        Vector2 size = m_loader->rectTransform()->size();
        camera->transform()->setPosition(Vector3(size.x * 0.5f, size.y * 0.5f, 1.0f));
    }

    connect(m_controller, &WidgetController::objectsSelected, this, &UiEdit::objectsSelected);
    connect(m_controller, &WidgetController::sceneUpdated, this, &UiEdit::updated);

    auto groups = componentGroups();
    std::string group = groups.back().toStdString();
    for(auto &it : Engine::factories()) {
        if(it.second.find(group) != std::string::npos) {
            Actor *actor = Engine::composeActor(it.first, it.first);
            if(actor) {
                Widget *widget = dynamic_cast<Widget *>(actor->component(it.first));
                if(widget) {
                    m_widgets[it.first] = widget;
                }
            }
        }
    }
}

UiEdit::~UiEdit() {
    delete ui;
}

bool UiEdit::isModified() const {
    return (UndoManager::instance()->lastCommand(this) != m_lastCommand);
}

QStringList UiEdit::suffixes() const {
    return { "ui" };
}

QStringList UiEdit::componentGroups() const {
    return {"Actor", "Components/UI"};
}

void UiEdit::onActivated() {
    emit objectsHierarchyChanged(m_scene);

    emit objectsSelected(m_controller->selected());
}

void UiEdit::onUpdated() {
    emit updated();
}

void UiEdit::onObjectCreate(QString type) {
    UndoManager::instance()->push(new CreateObject(type, m_scene, m_controller));
}

void UiEdit::onObjectsSelected(QList<Object *> objects, bool force) {
    m_controller->onSelectActor(objects);
}

void UiEdit::onObjectsDeleted(QList<Object *> objects) {
    UndoManager::instance()->push(new DeleteObject(objects, m_controller));
}

void UiEdit::onObjectsChanged(const QList<Object *> &objects, QString property, const Variant &value) {
    for(auto object : objects) {
        const MetaObject *meta = object->metaObject();

        int32_t index = meta->indexOfProperty(qPrintable(property));
        if(index > -1) {
            MetaProperty property = meta->property(index);

            std::string tag(propertyTag(property, gCss));
            if(!tag.empty()) {
                std::string editor(propertyTag(property, gEditorTag));

                std::string data;

                switch(value.userType()) {
                    case MetaType::BOOLEAN: {
                        bool v = value.toBool();
                        if(tag == "white-space") {
                            data = v ? "normal" : "nowrap";
                        } else if(tag == "font-kerning") {
                            data = v ? "normal" : "none";
                        }
                    } break;
                    case MetaType::INTEGER: {
                        if(tag == "text-align") {
                            value.toInt();
                        } else {
                            data += std::to_string(value.toInt()) + "px";
                        }
                    } break;
                    case MetaType::FLOAT: {
                        data += std::to_string(value.toFloat()) + "px";
                    } break;
                    case MetaType::VECTOR2: {
                        Vector2 v = value.toVector2();
                        for(uint32_t i = 0; i < 2; i++) {
                            data += std::to_string(v[i]) + "px ";
                        }
                        data.pop_back();
                    } break;
                    case MetaType::VECTOR3: {
                        Vector3 v = value.toVector3();
                        for(uint32_t i = 0; i < 3; i++) {
                            data += std::to_string(v[i]) + "px ";
                        }
                        data.pop_back();
                    } break;
                    case MetaType::VECTOR4: {
                        Vector4 v = value.toVector4();
                        if(editor == "Color") {
                            std::stringstream ss;
                            ss << "#";

                            for(uint32_t i = 0; i < 4; i++) {
                                uint32_t c = v[i] * 255.0f;

                                if(c == 0) {
                                    ss << "00";
                                } else {
                                    ss << std::hex << c;
                                }
                            }

                            data = ss.str();
                        } else {
                            for(uint32_t i = 0; i < 4; i++) {
                                data += std::to_string(v[i]) + "px ";
                            }
                            data.pop_back();
                        }
                    } break;
                }

                Widget *widget = dynamic_cast<Widget *>(object);
                if(widget) {
                    StyleSheet::setStyleProperty(widget, tag, data);
                }
            }
        }
    }

    QString capital = property;
    capital[0] = capital[0].toUpper();
    QString name(QObject::tr("Change %1").arg(capital));

    UndoManager::instance()->push(new ChangeProperty(objects, property, value, m_controller, name));
}

void UiEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        m_lastCommand = UndoManager::instance()->lastCommand(this);

        QFile loadFile(settings->source());
        if(!loadFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open file.");
            return;
        }

        QByteArray data(loadFile.readAll());
        loadFile.close();

        m_loader->fromBuffer(data.toStdString());
    }
}

void UiEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_settings.first()->source().isEmpty()) {

        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child(gUi);

        std::string style = m_loader->documentStyle();
        if(!style.empty()) {
            pugi::xml_node styleNode = root.append_child(gStyle);
            styleNode.set_value(style.c_str());
        }

        saveElementHelper(root, m_loader);

        std::stringstream ss;
        doc.save(ss);

        QFile loadFile(m_settings.front()->source());
        if(!loadFile.open(QIODevice::WriteOnly)) {
            qWarning("Couldn't open file.");
            return;
        }

        loadFile.write(ss.str().c_str());
        loadFile.close();

        m_lastCommand = UndoManager::instance()->lastCommand(this);
    }
}

void UiEdit::saveElementHelper(pugi::xml_node &parent, Widget *widget) {
    for(auto it : widget->childWidgets()) {
        if(widget->isSubWidget(it)) {
            continue;
        }
        auto originIt = m_widgets.find(it->typeName());
        if(originIt != m_widgets.end()) {
            pugi::xml_node element = parent.append_child(it->typeName().c_str());
            element.append_attribute(gName).set_value(it->actor()->name().c_str());

            std::string style = it->style();
            if(!style.empty()) {
                element.append_attribute(gStyle).set_value(style.c_str());
            }

            std::string classes;
            for(auto &classIt : it->classes()) {
                classes += classIt + ' ';
            }

            if(!classes.empty()) {
                classes.pop_back();
                element.append_attribute(gClass).set_value(classes.c_str());
            }

            const MetaObject *meta = it->metaObject();
            for(uint32_t i = 0; i < meta->propertyCount(); i++) {
                MetaProperty property = meta->property(i);

                std::string tag(propertyTag(property, gCss));
                if(!tag.empty()) {
                    continue;
                }

                Variant origin = originIt->second->property(property.name());
                Variant current = it->property(property.name());

                if(origin != current) {
                    switch(current.type()) {
                    case MetaType::BOOLEAN:
                    case MetaType::INTEGER:
                    case MetaType::FLOAT:
                    case MetaType::STRING: element.append_attribute(property.name()).set_value(current.toString().c_str()); break;
                    default: break;
                    }
                }
            }

            saveElementHelper(element, it);
        }

    }
}

void UiEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

std::string UiEdit::propertyTag(const MetaProperty &property, const std::string &tag) const {
    if(property.table() && property.table()->annotation) {
        std::string annotation(property.table()->annotation);

        for(auto it : StringUtil::split(annotation, ',')) {
            it = StringUtil::deletechar(it, ' ');
            if(StringUtil::contains(it, tag)) {
                StringUtil::replace(it, tag, "");
                return it;
            }
        }
    }
    return std::string();
}
