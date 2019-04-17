#include "timeline.h"
#include "ui_timeline.h"

#include <timer.h>
#include <json.h>

#include <resources/animationclip.h>
#include <components/animationcontroller.h>
#include <components/actor.h>

#include "animationclipmodel.h"

#include "assetmanager.h"
#include "projectmanager.h"

#include <QSettings>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQmlEngine>
#include <QQuickItem>
#include <QJsonDocument>
#include <QJsonArray>

bool compare(const AnimationClip::Track &first, const AnimationClip::Track &second) {
    if(first.path == second.path) {
        return first.property < second.property;
    }
    return first.path < second.path;
}

Timeline::Timeline(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Timeline),
        m_pController(nullptr),
        m_TimerId(0),
        m_ContentMenu(this),
        m_Modified(false),
        m_pKey(nullptr),
        m_Row(-1),
        m_Col(-1),
        m_Ind(-1) {
    ui->setupUi(this);
    ui->pause->setVisible(false);

    readSettings();

    m_pModel = new AnimationClipModel(this);

    ui->treeView->setModel(m_pModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setMouseTracking(true);

    ui->quickTimeline->rootContext()->setContextProperty("clipModel", m_pModel);
    ui->quickTimeline->setSource(QUrl("qrc:/QML/qml/Timeline.qml"));

    ui->record->setProperty("checkred", true);
    ui->play->setProperty("checkgreen", true);
    ui->curve->setProperty("checkgreen", true);

    QQuickItem *item = ui->quickTimeline->rootObject();
    connect(item, SIGNAL(addKey(int,int,int)), m_pModel, SLOT(onAddKey(int,int,int)));
    connect(item, SIGNAL(removeKey(int,int,int)), m_pModel, SLOT(onRemoveKey(int,int,int)));
    connect(item, SIGNAL(selectKey(int,int,int)), this, SLOT(onSelectKey(int,int,int)));

    connect(m_pModel, &AnimationClipModel::changed, this, &Timeline::onModified);
    connect(m_pModel, &AnimationClipModel::positionChanged, this, &Timeline::moved);

    connect(ui->valueEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);
    connect(ui->timeEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);

    connect(ui->clipBox, SIGNAL(activated(QString)), m_pModel, SLOT(setClip(QString)));

    m_ContentMenu.addAction(tr("Remove Properties"), this, SLOT(onRemoveProperty()));
}

Timeline::~Timeline() {
    saveClip();

    writeSettings();

    delete ui;
}

void Timeline::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value  = settings.value("timeline.splitter");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void Timeline::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("timeline.splitter", ui->splitter->saveState());
}

void Timeline::saveClip() {
    AnimationClip *clip = m_pModel->clip();
    if(m_Modified && clip) {
        string ref  = Engine::reference(clip);
        QFile file(ProjectManager::instance()->contentPath() + "/" + AssetManager::instance()->guidToPath(ref).c_str());
        if(file.open(QIODevice::WriteOnly)) {
            QVariantList tracks;
            for(auto t : clip->m_Tracks) {
                QVariantList track;
                track.push_back(t.path.c_str());
                track.push_back(t.property.c_str());

                QVariantList curves;
                for(auto c : t.curves) {
                    QVariantList keys;
                    keys.push_back(c.first);
                    for(auto k : c.second.m_Keys) {
                        QVariantList key;
                        key.push_back(k.m_Position);
                        key.push_back(k.m_Type);
                        key.push_back(k.m_Value);
                        key.push_back(k.m_LeftTangent);
                        key.push_back(k.m_RightTangent);

                        keys.push_back(key);
                    }
                    curves.push_back(keys);
                }
                track.push_back(curves);
                tracks.push_back(track);
            }

            QJsonDocument doc(QJsonArray::fromVariantList(tracks));
            file.write(doc.toJson());
            file.close();

            m_Modified  = false;
        }

    }
}

AnimationController *Timeline::findController(Object *object) {
    AnimationController *result = object->findChild<AnimationController *>(false);
    if(!result) {
        Object *parent  = object->parent();
        if(parent) {
            result  = findController(parent);
        }
    }
    return result;
}

void Timeline::onObjectSelected(Object::ObjectList objects) {
    if(objects.empty()) {
        return;
    }

    if(m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId   = 0;
    }

    saveClip();

    m_pController   = nullptr;
    for(auto object : objects) {
        m_pController  = findController(object);
        if(m_pController) {
            break;
        }
    }
    m_pModel->setController(m_pController);
    ui->clipBox->clear();
    ui->clipBox->addItems(m_pModel->clips());

    bool enable = (m_pController != nullptr);

    ui->begin->setEnabled(enable);
    ui->end->setEnabled(enable);

    ui->record->setEnabled(enable);
    ui->record->setChecked(false);

    ui->play->setEnabled(enable);
    ui->play->setChecked(false);

    ui->next->setEnabled(enable);
    ui->previous->setEnabled(enable);

    ui->treeView->setCurrentIndex(m_pModel->index(0, 0));

    onSelectKey(-1, -1, -1);

    emit animated(enable);

    //m_pModel->blockSignals(true);
    on_begin_clicked();
    //m_pModel->blockSignals(false);
}

void Timeline::onChanged(Object::ObjectList objects, const QString &property) {
    for(auto it : objects) {
        onUpdated(it, property);
    }
}

void Timeline::onUpdated(Object *object, const QString &property) {
    m_pModel->setController(m_pController);
    ui->clipBox->addItems(m_pModel->clips());

    if(object && !property.isEmpty() && ui->record->isChecked()) {
        AnimationController *controller = findController(object);
        if(controller) {
            AnimationClip *clip = m_pModel->clip();
            if(clip == nullptr) {
                return;
            }

            QString path    = pathTo(static_cast<Object *>(controller->actor()), object);

            const MetaObject *meta  = object->metaObject();
            int32_t index   = meta->indexOfProperty(qPrintable(property));
            if(index >= 0) {
                MetaProperty p  = meta->property(index);
                Variant value   = p.read(object);

                vector<float> data;
                switch(value.type()) {
                    case MetaType::VECTOR2: {
                        Vector2 v = value.toVector2();
                        data = {v.x, v.y};
                    } break;
                    case MetaType::VECTOR3: {
                        Vector3 v = value.toVector3();
                        data = {v.x, v.y, v.z};
                    } break;
                    case MetaType::VECTOR4: {
                        Vector4 v = value.toVector4();
                        data = {v.x, v.y, v.z, v.w};
                    } break;
                    default: {
                        data = {value.toFloat()};
                    } break;
                }

                for(uint32_t component = 0; component < data.size(); component++) {
                    bool create = true;

                    AnimationCurve::KeyFrame key;
                    key.m_Position = controller->position();
                    key.m_Value = data[component];
                    key.m_LeftTangent  = key.m_Value;
                    key.m_RightTangent  = key.m_Value;

                    for(auto &it : clip->m_Tracks) {
                        if(it.path == path.toStdString() && it.property == property.toStdString()) {
                            bool update = false;

                            auto &curve = it.curves[component];
                            for(auto &k : curve.m_Keys) {

                                if(k.m_Position == key.m_Position) {
                                    k.m_Value = key.m_Value;
                                    k.m_LeftTangent = key.m_LeftTangent;
                                    k.m_RightTangent = key.m_RightTangent;
                                    update  = true;
                                }
                            }
                            if(!update) {
                                curve.m_Keys.push_back(key);
                                std::sort(curve.m_Keys.begin(), curve.m_Keys.end(), AnimationClip::compare);
                            }
                            create  = false;
                            break;
                        }
                    }
                    if(create) {
                        AnimationClip::Track track;
                        track.path = path.toStdString();
                        track.property = property.toStdString();

                        AnimationCurve curve;
                        curve.m_Keys.push_back(key);

                        track.curves[component] = curve;

                        clip->m_Tracks.push_back(track);
                        clip->m_Tracks.sort(compare);
                    }
                }

                m_pModel->setController(m_pController);
                m_pModel->updateController();
                ui->clipBox->addItems(m_pModel->clips());
                onModified();
            }
        }
    }
}

void Timeline::onModified() {
    m_Modified  = true;
}

void Timeline::onRemoveProperty() {
    QModelIndexList list = ui->treeView->selectionModel()->selectedIndexes();
    AnimationClip *clip = m_pModel->clip();
    foreach(const QModelIndex &index, list) {
        auto it = clip->m_Tracks.begin();
        advance(it, index.row());

        clip->m_Tracks.erase(it);
    }
    m_pModel->setController(m_pController);
    m_pModel->updateController();

    onModified();
}

void Timeline::onSelectKey(int row, int col, int index) {
    m_Row = row;
    m_Col = col;
    m_Ind = index;

    m_pKey = m_pModel->key(row, col, index);
    if(m_pKey) {
        ui->timeEdit->setText(QString::number(m_pKey->m_Position));
        ui->valueEdit->setText(QString::number(m_pKey->m_Value));
    }
    ui->deleteKey->setEnabled((m_pKey != nullptr));
}

void Timeline::onKeyChanged() {
    if(m_pKey) {
        m_pKey->m_Position = ui->timeEdit->text().toUInt();

        float delta = ui->valueEdit->text().toFloat() - m_pKey->m_Value;

        m_pKey->m_Value += delta;
        m_pKey->m_LeftTangent += delta;
        m_pKey->m_RightTangent += delta;

        m_pModel->updateController();
    }
}

void Timeline::timerEvent(QTimerEvent *) {
    if(m_pController) {
        uint32_t ms = m_pController->position() + static_cast<uint32_t>(Timer::deltaTime() * 1000.0f);
        if(ms >= m_pController->duration()) {
            on_begin_clicked();
        } else {
            m_pModel->setPosition(ms / 1000.0f);
        }
    }
}

void Timeline::on_play_clicked() {
    if(m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId   = 0;
    } else {
        m_TimerId   = startTimer(16);
    }
}

void Timeline::on_begin_clicked() {
    m_pModel->setPosition(0.0f);
}

void Timeline::on_end_clicked() {
    m_pModel->setPosition(m_pController->duration() / 1000.0f);
}

void Timeline::on_previous_clicked() {
    m_pModel->setPosition(findNear(true) / 1000.0f);
}

void Timeline::on_next_clicked() {
    m_pModel->setPosition(findNear() / 1000.0f);
}

uint32_t Timeline::findNear(bool backward) {
    uint32_t result = 0;
    if(m_pController) {
        uint32_t current = m_pController->position();
        AnimationClip *clip = m_pModel->clip();
        if(clip) {
            if(backward) {
                result = 0;
                for(auto it : clip->m_Tracks) {
                    for(auto c : it.curves) {
                        auto key = c.second.m_Keys.rbegin();
                        while(key != c.second.m_Keys.rend()) {
                            if(key->m_Position < current) {
                                result = max(result, key->m_Position);
                                break;
                            }
                            key++;
                        }
                    }
                }
            } else {
                result = UINT_MAX;
                for(auto it : clip->m_Tracks) {
                    for(auto c : it.curves) {
                        for(auto key : c.second.m_Keys) {
                            if(key.m_Position > current) {
                                result = min(result, key.m_Position);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

QString Timeline::pathTo(Object *src, Object *dst) {
    QString result;
    if(src != dst) {
        QString parent  = pathTo(src, dst->parent());
        if(!parent.isEmpty()) {
            result  += parent + "/";
        }
        result  += dst->name().c_str();
    }

    return result;
}

void Timeline::on_treeView_customContextMenuRequested(const QPoint &pos) {
    if(!ui->treeView->selectionModel()->selectedIndexes().empty()) {
        m_ContentMenu.exec(ui->treeView->mapToGlobal(pos));
    }
}

void Timeline::on_treeView_clicked(const QModelIndex &index) {
    m_pModel->selectItem(index);
}

void Timeline::on_curve_toggled(bool checked) {
    QObject *curve = ui->quickTimeline->rootObject()->findChild<QObject *>("curve");
    if(curve) {
        QQmlProperty::write(curve, "visible", checked);
    }
    QObject *keys = ui->quickTimeline->rootObject()->findChild<QObject *>("keys");
    if(keys) {
        QQmlProperty::write(keys, "visible", !checked);
    }
}

void Timeline::on_flatKey_clicked() {
    if(m_pKey) {
        m_pKey->m_LeftTangent = m_pKey->m_Value;
        m_pKey->m_RightTangent = m_pKey->m_Value;

        m_pModel->updateController();
    }
}

void Timeline::on_breakKey_clicked() {
    if(m_pKey) {
        m_pKey->m_LeftTangent = m_pKey->m_Value;
        m_pKey->m_RightTangent = m_pKey->m_Value;

        m_pModel->changed();
    }
}

void Timeline::on_deleteKey_clicked() {
    m_pModel->onRemoveKey(m_Row, m_Col, m_Ind);
}
