#include "timeline.h"
#include "ui_timeline.h"

#include <json.h>

#include <resources/animationclip.h>
#include <components/animationcontroller.h>
#include <components/actor.h>

#include "animationclipmodel.h"

#include "keyframeeditor.h"

#include "assetmanager.h"
#include "projectmanager.h"

#include <QSettings>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQmlEngine>
#include <QQuickItem>
#include <QJsonDocument>
#include <QJsonArray>

bool compareTracks(const AnimationTrack &first, const AnimationTrack &second) {
    if(first.path() == second.path()) {
        return first.property() < second.property();
    }
    return first.path() < second.path();
}

bool compareKeys(const AnimationCurve::KeyFrame &first, const AnimationCurve::KeyFrame &second) {
    return ( first.m_Position < second.m_Position );
}

Timeline::Timeline(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Timeline),
        m_pController(nullptr),
        m_TimerId(0),
        m_Row(-1),
        m_Col(-1),
        m_Ind(-1),
        m_Modified(false) {

    ui->setupUi(this);
    ui->pause->setVisible(false);

    m_pModel = new AnimationClipModel(this);

    ui->record->setProperty("checkred", true);
    ui->play->setProperty("checkgreen", true);
    ui->curve->setProperty("checkgreen", true);

    ui->widget->setModel(m_pModel);

    connect(m_pModel, &AnimationClipModel::changed, this, &Timeline::onModified);
    connect(m_pModel, &AnimationClipModel::positionChanged, this, &Timeline::moved);

    connect(ui->valueEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);
    connect(ui->timeEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);

    connect(ui->clipBox, SIGNAL(activated(QString)), m_pModel, SLOT(setClip(QString)));

    connect(ui->widget, &KeyFrameEditor::keySelectionChanged, this, &Timeline::onSelectKey);

    ui->toolBar->hide();
}

Timeline::~Timeline() {
    saveClip();

    delete ui;
}

void Timeline::saveClip() {
    AnimationClip *clip = m_pModel->clip();
    if(m_Modified && clip) {
        VariantMap data = clip->saveUserData();

        string ref  = Engine::reference(clip);
        QFile file(ProjectManager::instance()->contentPath() + "/" + AssetManager::instance()->guidToPath(ref).c_str());
        if(file.open(QIODevice::WriteOnly)) {
            file.write(Json::save(data["Tracks"], 0).c_str());
            file.close();

            m_Modified  = false;
        }
    }
}

AnimationController *Timeline::findController(Object *object) {
    AnimationController *result = object->findChild<AnimationController *>(false);
    if(!result) {
        Object *parent = object->parent();
        if(parent) {
            result = findController(parent);
        }
    }
    return result;
}

void Timeline::onObjectSelected(Object::ObjectList objects) {
    if(m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId   = 0;
    }

    saveClip();

    m_SelectedObjects = objects;

    AnimationController *result = nullptr;
    for(auto object : m_SelectedObjects) {
        result  = findController(object);
        if(result) {
            break;
        }
    }
    if(m_pController != result) {
        m_pController = result;

        bool enable = (m_pController != nullptr);

        ui->begin->setEnabled(enable);
        ui->end->setEnabled(enable);

        ui->record->setEnabled(enable);
        ui->record->setChecked(false);

        ui->play->setEnabled(enable);
        ui->play->setChecked(false);

        ui->next->setEnabled(enable);
        ui->previous->setEnabled(enable);

        emit animated(enable);

        updateClips();
    }
}

void Timeline::updateClips() {
    m_pModel->setController(m_pController);
    ui->clipBox->clear();
    ui->clipBox->addItems(m_pModel->clips());

    onSelectKey(-1, -1, -1);

    on_begin_clicked();
}

void Timeline::onChanged(Object::ObjectList objects, const QString property) {
    for(auto it : objects) {
        onUpdated(it, property);
    }
}

void Timeline::showBar() {
    ui->toolBar->show();
}

void Timeline::onUpdated(Object *object, const QString property) {
    if(object) {
        if(object == m_pController) {
            updateClips();
            return;
        }
        if(!property.isEmpty() && ui->record->isChecked()) {
            if(m_pController) {
                AnimationClip *c = m_pModel->clip();
                if(c == nullptr) {
                    return;
                }

                QString path = pathTo(static_cast<Object *>(m_pController->actor()), object);

                const MetaObject *meta = object->metaObject();
                int32_t index = meta->indexOfProperty(qPrintable(property));
                if(index >= 0) {
                    MetaProperty p = meta->property(index);
                    Variant value = p.read(object);

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

                    AnimationTrackList tracks = c->m_Tracks;

                    for(uint32_t component = 0; component < data.size(); component++) {
                        bool create = true;

                        AnimationCurve::KeyFrame key;
                        key.m_Position = m_pController->position();
                        key.m_Value = data[component];
                        key.m_LeftTangent = key.m_Value;
                        key.m_RightTangent = key.m_Value;

                        for(auto &it : tracks) {
                            if(it.path() == path.toStdString() && it.property() == property.toStdString()) {
                                bool update = false;

                                auto &curve = it.curves()[component];
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
                                    std::sort(curve.m_Keys.begin(), curve.m_Keys.end(), compareKeys);
                                    it.setDuration(MAX(it.duration(), (int)m_pController->position()));
                                }
                                create = false;
                                break;
                            }
                        }
                        if(create) {
                            AnimationTrack track;
                            track.setPath(path.toStdString());
                            track.setProperty(property.toStdString());
                            track.setDuration(m_pController->position());

                            AnimationCurve curve;
                            curve.m_Keys.push_back(key);

                            track.curves()[component] = curve;

                            tracks.push_back(track);
                            tracks.sort(compareTracks);
                        }
                    }
                    UndoManager::instance()->push(new UndoUpdateItems(tracks, m_pModel, tr("Update Properties")));
                }
            }
        }
    }
}

void Timeline::onModified() {
    m_Modified = true;
}

void Timeline::onSelectKey(int row, int col, int index) {
    m_Row = row;
    m_Col = col;
    m_Ind = index;

    ui->valueEdit->clear();
    ui->timeEdit->clear();

    if(row > -1 && col == -1) {
        col = 0;
    }
    AnimationCurve::KeyFrame *key = m_pModel->key(row, col, index);
    if(key) {
        AnimationTrack &t = m_pModel->track(row);
        if(m_Col > -1) {
            ui->valueEdit->setText(QString::number(key->m_Value));
        }
        ui->timeEdit->setText(QString::number(key->m_Position * t.duration()));
    }
    ui->deleteKey->setEnabled(key != nullptr);
}

void Timeline::onKeyChanged() {
    AnimationCurve::KeyFrame *key = m_pModel->key(m_Row, m_Col, m_Ind);
    if(key) {
        float delta = ui->valueEdit->text().toFloat() - key->m_Value;
        m_pModel->commitKey(m_Row, m_Col, m_Ind, key->m_Value + delta,
                                                 key->m_LeftTangent + delta,
                                                 key->m_RightTangent + delta, ui->timeEdit->text().toUInt());
    }
}

void Timeline::timerEvent(QTimerEvent *) {
    AnimationClip *clip = m_pModel->clip();
    if(m_pController && clip) {
        int32_t ms = m_pController->position() + static_cast<int32_t>(0.0625f * 1000.0f);
        if(ms >= clip->duration()) {
            on_begin_clicked();
        } else {
            m_pModel->setPosition(ms);
        }
    }
}

void Timeline::on_play_clicked() {
    if(m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId = 0;
    } else {
        m_TimerId = startTimer(16);
    }
}

void Timeline::on_begin_clicked() {
    m_pModel->setPosition(0.0f);
}

void Timeline::on_end_clicked() {
    AnimationClip *clip = m_pModel->clip();
    if(clip) {
        m_pModel->setPosition(clip->duration());
    }
}

void Timeline::on_previous_clicked() {
    m_pModel->setPosition(findNear(true));
}

void Timeline::on_next_clicked() {
    m_pModel->setPosition(findNear());
}

float Timeline::findNear(bool backward) {
    float result = 0;
    if(m_pController) {
        float current = m_pModel->position();
        AnimationClip *clip = m_pModel->clip();
        if(clip) {
            if(backward) {
                result = 0;
                for(auto it : clip->m_Tracks) {
                    for(auto c : it.curves()) {
                        auto key = c.second.m_Keys.rbegin();
                        while(key != c.second.m_Keys.rend()) {
                            float pos = key->m_Position * clip->duration();
                            if(pos < current) {
                                result = MAX(result, pos);
                                break;
                            }
                            key++;
                        }
                    }
                }
            } else {
                result = clip->duration();
                for(auto it : clip->m_Tracks) {
                    for(auto c : it.curves()) {
                        for(auto key : c.second.m_Keys) {
                            float pos = key.m_Position * clip->duration();
                            if(pos > current) {
                                result = MIN(result, pos);
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

void Timeline::on_flatKey_clicked() {
    AnimationCurve::KeyFrame *key = m_pModel->key(m_Row, m_Col, m_Ind);
    if(key) {
        m_pModel->commitKey(m_Row, m_Col, m_Ind, key->m_Value, key->m_Value, key->m_Value, key->m_Position);
    }
}

void Timeline::on_breakKey_clicked() {

}

void Timeline::on_deleteKey_clicked() {
    ui->widget->onDeleteSelectedKey();
}

void Timeline::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
