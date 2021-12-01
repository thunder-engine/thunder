#include "timeline.h"
#include "ui_timeline.h"

#include <json.h>

#include <resources/animationstatemachine.h>
#include <resources/animationclip.h>
#include <components/animator.h>
#include <components/actor.h>

#include "animationclipmodel.h"

#include "keyframeeditor.h"

#include "assetmanager.h"

Timeline::Timeline(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Timeline),
        m_controller(nullptr),
        m_model(new AnimationClipModel(this)),
        m_TimerId(0),
        m_Row(-1),
        m_Col(-1),
        m_Ind(-1),
        m_Modified(false) {

    ui->setupUi(this);
    ui->pause->setVisible(false);

    setController(nullptr);

    ui->record->setProperty("checkred", true);
    ui->play->setProperty("checkgreen", true);
    ui->curve->setProperty("checkgreen", true);

    ui->widget->setModel(m_model);

    connect(m_model, &AnimationClipModel::changed, this, &Timeline::onModified);
    connect(m_model, &AnimationClipModel::rebind, this, &Timeline::onRebind);

    connect(ui->valueEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);
    connect(ui->timeEdit, &QLineEdit::editingFinished, this, &Timeline::onKeyChanged);

    connect(ui->clipBox, SIGNAL(activated(QString)), this, SLOT(onClipChanged(QString)));

    connect(ui->widget, &KeyFrameEditor::keySelectionChanged, this, &Timeline::onSelectKey);
    connect(ui->widget, &KeyFrameEditor::rowsSelected, this, &Timeline::onRowsSelected);
    connect(ui->widget, &KeyFrameEditor::headPositionChanged, this, &Timeline::setPosition);

    connect(ui->deleteKey, &QToolButton::clicked, ui->widget, &KeyFrameEditor::onDeleteSelectedKey);

    ui->toolBar->hide();
}

Timeline::~Timeline() {
    saveClip();

    delete ui;
}

void Timeline::saveClip() {
    AnimationClip *clip = m_model->clip();
    if(m_Modified && clip) {
        VariantMap data = clip->saveUserData();

        string ref = Engine::reference(clip);
        QFile file(AssetManager::instance()->guidToPath(ref).c_str());
        if(file.open(QIODevice::WriteOnly)) {
            file.write(Json::save(data["Tracks"], 0).c_str());
            file.close();

            m_Modified = false;
        }
    }
}

Animator *Timeline::findController(Object *object) {
    Animator *result = object->findChild<Animator *>(false);
    if(!result) {
        Object *parent = object->parent();
        if(parent) {
            result = findController(parent);
        }
    }
    return result;
}

void Timeline::onObjectsSelected(Object::ObjectList objects) {
    if(m_TimerId) {
        killTimer(m_TimerId);
        m_TimerId   = 0;
    }

    Animator *result = nullptr;
    for(auto object : objects) {
        result = findController(object);
        if(result) {
            break;
        }
    }
    setController(result);
}

void Timeline::updateClips() {
    ui->clipBox->clear();
    ui->clipBox->addItems(m_clips.keys());

    onSelectKey(-1, -1, -1);

    on_begin_clicked();
}

uint32_t Timeline::position() const {
    if(m_controller) {
        return m_controller->position();
    }
    return 0;
}

void Timeline::setPosition(uint32_t position) {
    if(m_controller) {
        m_controller->setPosition(position);
    }
    // This method is very heavy
    ui->widget->setPosition(position);

    emit moved();
}

void Timeline::setController(Animator *controller) {
    if(m_controller != controller) {
        saveClip();

        m_clips.clear();
        m_controller = controller;
        if(m_controller) {
            AnimationStateMachine *stateMachine = controller->stateMachine();
            if(stateMachine) {
                for(auto it : stateMachine->states()) {
                    QFileInfo info(AssetManager::instance()->guidToPath(Engine::reference(it->m_clip)).c_str());
                    m_clips[info.baseName()] = it->m_clip;
                }
                if(!m_clips.isEmpty()) {
                    m_currentClip = m_clips.begin().key();
                    m_model->setClip(m_clips.begin().value(), controller->actor());
                    m_controller->setClip(m_clips.begin().value());
                }
            }
        } else {
            m_currentClip.clear();
            m_model->setClip(nullptr, nullptr);
        }

        bool enable = (m_controller != nullptr);

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

void Timeline::onObjectsChanged(Object::ObjectList objects, const QString property) {
    for(auto it : objects) {
        onPropertyUpdated(it, property);
    }
}

void Timeline::showBar() {
    ui->toolBar->show();
}

void Timeline::onPropertyUpdated(Object *object, const QString property) {
    if(object) {
        if(object == m_controller) {
            updateClips();
            return;
        }
        if(!m_model->isReadOnly() && !property.isEmpty() && ui->record->isChecked()) {
            if(m_controller) {
                AnimationClip *clip = m_model->clip();
                if(clip == nullptr) {
                    return;
                }

                QString path = pathTo(static_cast<Object *>(m_controller->actor()), object);
                m_model->propertyUpdated(object, path, property, position());
            }
        }
    }
}

void Timeline::onModified() {
    m_Modified = true;

}

void Timeline::onRebind() {
    m_controller->rebind();
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
    AnimationCurve::KeyFrame *key = m_model->key(row, col, index);
    if(key) {
        AnimationTrack &t = m_model->track(row);
        if(m_Col > -1) {
            ui->valueEdit->setText(QString::number(key->m_Value));
        }
        ui->timeEdit->setText(QString::number(key->m_Position * t.duration()));
    }
    ui->deleteKey->setEnabled(key != nullptr);
}

void Timeline::onRowsSelected(QStringList list) {
    Object::ObjectList result;
    for(auto &it : list) {
        Object *object = m_controller->actor()->find(it.toStdString());
        if(object) {
            Component *component = dynamic_cast<Component *>(object);
            if(component) {
                result.push_back(component->actor());
            }
        }
    }
    if(!result.empty()) {
        emit objectSelected(result);
    }
}

void Timeline::onClipChanged(const QString &clip) {
    m_currentClip = clip;
    m_model->setClip(m_clips.value(m_currentClip), m_controller->actor());
    m_controller->setClip(m_clips.value(m_currentClip));
}

void Timeline::onKeyChanged() {
    AnimationCurve::KeyFrame *key = m_model->key(m_Row, m_Col, m_Ind);
    if(key) {
        float delta = ui->valueEdit->text().toFloat() - key->m_Value;
        m_model->commitKey(m_Row, m_Col, m_Ind, key->m_Value + delta,
                                                 key->m_LeftTangent + delta,
                                                 key->m_RightTangent + delta, ui->timeEdit->text().toUInt());
    }
}

void Timeline::timerEvent(QTimerEvent *) {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        int32_t ms = position() + 60;
        if(ms >= clip->duration()) {
            on_begin_clicked();
        } else {
            setPosition(ms);
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
    setPosition(0);
}

void Timeline::on_end_clicked() {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        setPosition(clip->duration());
    }
}

void Timeline::on_previous_clicked() {
    setPosition(m_model->findNear(position(), true));
}

void Timeline::on_next_clicked() {
    setPosition(m_model->findNear(position()));
}

QString Timeline::pathTo(Object *src, Object *dst) {
    QString result;
    if(src != dst) {
        QString parent = pathTo(src, dst->parent());
        if(!parent.isEmpty()) {
            result += parent + "/";
        }
        result += dst->name().c_str();
    }

    return result;
}

void Timeline::on_flatKey_clicked() {
    AnimationCurve::KeyFrame *key = m_model->key(m_Row, m_Col, m_Ind);
    if(key) {
        m_model->commitKey(m_Row, m_Col, m_Ind, key->m_Value, key->m_Value, key->m_Value, key->m_Position);
    }
}

void Timeline::on_breakKey_clicked() {

}

void Timeline::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
