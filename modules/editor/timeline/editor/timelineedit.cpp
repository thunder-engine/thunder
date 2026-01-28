#include "timelineedit.h"
#include "ui_timelineedit.h"

#include <json.h>

#include <resources/animationstatemachine.h>
#include <resources/animationclip.h>
#include <components/animator.h>
#include <components/actor.h>

#include "animationclipmodel.h"

#include "keyframeeditor.h"

#include <assetmanager.h>

TimelineEdit::TimelineEdit(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::TimelineEdit),
        m_model(new AnimationClipModel(this)) {

    ui->setupUi(this);
    ui->pause->setVisible(false);

    ui->value->setVisible(false);
    ui->valueEdit->setVisible(false);

    setController(nullptr);

    ui->record->setProperty("checkred", true);
    ui->play->setProperty("checkgreen", true);
    ui->splineMode->setProperty("checkgreen", true);

    ui->widget->setModel(m_model);

    ui->timeEdit->setValidator(new QIntValidator(0, INT32_MAX, this));

    connect(m_model, &AnimationClipModel::rebind, this, &TimelineEdit::onRebind);

    connect(ui->valueEdit, &QLineEdit::editingFinished, this, &TimelineEdit::onKeyChanged);
    connect(ui->timeEdit, &QLineEdit::editingFinished, this, &TimelineEdit::onKeyChanged);

    connect(ui->clipBox, &QComboBox::textActivated, this, &TimelineEdit::onClipChanged);

    connect(ui->widget, &KeyFrameEditor::keySelectionChanged, this, &TimelineEdit::onSelectKey);
    connect(ui->widget, &KeyFrameEditor::rowsSelected, this, &TimelineEdit::onRowsSelected);
    connect(ui->widget, &KeyFrameEditor::headPositionChanged, this, &TimelineEdit::setPosition);

    connect(ui->deleteKey, &QToolButton::clicked, ui->widget, &KeyFrameEditor::onDeleteSelectedKey);

    ui->toolBar->hide();
}

TimelineEdit::~TimelineEdit() {
    saveClip();

    delete ui;
}

void TimelineEdit::saveClip() {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        TString ref = Engine::reference(clip);
        File file(AssetManager::instance()->uuidToPath(ref));
        if(file.open(File::WriteOnly)) {
            VariantMap data = clip->saveUserData();

            file.write(Json::save(data["Tracks"], 0));
            file.close();
        }
    }
}

Animator *TimelineEdit::findController(Object *object) {
    Animator *result = object->findChild<Animator *>(false);
    if(!result) {
        Object *parent = object->parent();
        if(parent) {
            result = findController(parent);
        }
    }
    return result;
}

void TimelineEdit::onUpdated() {

}

void TimelineEdit::onObjectsSelected(const Object::ObjectList &objects) {
    if(m_timerId) {
        killTimer(m_timerId);
        m_timerId   = 0;
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

void TimelineEdit::updateClips() {
    m_clips.clear();

    if(m_controller) {
        AnimationStateMachine *stateMachine = m_controller->stateMachine();
        if(stateMachine) {
            for(auto it : stateMachine->states()) {
                TString ref = Engine::reference(it->m_clip);
                Url info(AssetManager::instance()->uuidToPath(ref).data());
                m_clips[info.baseName()] = it->m_clip;
            }
            if(!m_clips.empty()) {
                onClipChanged(m_clips.begin()->first.data());
            }
        } else {
            m_currentClip.clear();
            m_model->setClip(nullptr, nullptr);
            m_controller->setClip(nullptr);
        }
    } else {
        m_currentClip.clear();
        m_model->setClip(nullptr, nullptr);
    }

    ui->clipBox->clear();

    QStringList list;
    for(auto &it : m_clips) {
        list.push_back(it.first.data());
    }
    ui->clipBox->addItems(list);

    onSelectKey(-1, -1);

    on_begin_clicked();
}

uint32_t TimelineEdit::position() const {
    return m_position;
}

void TimelineEdit::setPosition(uint32_t position) {
    m_position = position;

    if(m_controller) {
        AnimationClip *clip = m_model->clip();
        m_controller->setClip(clip, m_position / clip->duration());
    }

    if(m_armature) {
        m_armature->update();
    }

    ui->widget->setPosition(position);
    ui->timeEdit->setText(QString::number(position));

    emit updated();
}

void TimelineEdit::setController(Animator *controller) {
    bool enable = (controller != nullptr);

    ui->begin->setEnabled(enable);
    ui->end->setEnabled(enable);

    ui->record->setEnabled(enable);
    ui->record->setChecked(false);

    ui->play->setEnabled(enable);
    ui->play->setChecked(false);

    ui->next->setEnabled(enable);
    ui->previous->setEnabled(enable);

    ui->deleteKey->setEnabled(false);
    ui->flatKey->setEnabled(false);

    if(m_controller != controller) {
        saveClip();

        m_controller = controller;
        m_armature = nullptr;

        if(m_controller) {
            m_armature = static_cast<NativeBehaviour *>(m_controller->actor()->componentInChild("Armature"));
        }

        ui->toolBar->setVisible(m_controller != nullptr);

        updateClips();
    }
}

void TimelineEdit::onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) {
    for(auto it : objects) {
        onPropertyUpdated(it, property);
    }
}

void TimelineEdit::onPropertyUpdated(Object *object, const TString &property) {
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

                TString path = pathTo(static_cast<Object *>(m_controller->actor()), object);
                m_model->propertyUpdated(object, path.data(), property.data(), position());
            }
        }
    }
}

void TimelineEdit::onRebind() {
    if(m_controller) {
        m_controller->rebind();
    }
}

void TimelineEdit::onSelectKey(int row, int index) {
    m_row = row;
    m_ind = index;

    ui->valueEdit->clear();
    ui->timeEdit->clear();

    AnimationCurve::KeyFrame *key = m_model->key(row, index);
    if(key) {
        AnimationTrack &t = m_model->track(row);
        ui->timeEdit->setText(QString::number(key->m_position * t.duration()));
    }
    ui->deleteKey->setEnabled(key != nullptr);
    ui->flatKey->setEnabled(key != nullptr);
}

void TimelineEdit::onRowsSelected(QStringList list) {
    std::list<Object *> result;
    if(m_controller) {
        for(auto &it : list) {
            Object *object = m_controller->actor()->find(it.toStdString());
            if(object) {
                Component *component = dynamic_cast<Component *>(object);
                if(component) {
                    result.push_back(component->actor());
                }
            }
        }
    }
    if(!result.empty()) {
        emit objectsSelected(result, false);
    }
}

void TimelineEdit::onClipChanged(const QString &clip) {
    m_currentClip = clip.toStdString();
    m_position = 0;
    if(m_controller) {
        auto it = m_clips.find(m_currentClip);
        if(it != m_clips.end()) {
            m_model->setClip(it->second, m_controller->actor());
            m_controller->setClip(it->second);
        }
    }
}

void TimelineEdit::onKeyChanged() {
    AnimationCurve::KeyFrame *key = m_model->key(m_row, m_ind);
    if(key) {
        float delta = ui->valueEdit->text().toFloat() - key->m_value.front();
        m_model->commitKey(m_row, m_ind, key->m_value.front() + delta,
                                         key->m_leftTangent.front() + delta,
                                         key->m_rightTangent.front() + delta, ui->timeEdit->text().toUInt());
    }
}

void TimelineEdit::timerEvent(QTimerEvent *) {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        int32_t ms = position() + 60;
        setPosition(ms);
        if(ms >= clip->duration()) {
            on_begin_clicked();
        }
    }
}

void TimelineEdit::on_play_clicked() {
    if(m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    } else {
        m_timerId = startTimer(16);
    }
}

void TimelineEdit::on_begin_clicked() {
    if(m_controller) {
        auto it = m_clips.find(m_currentClip);
        if(it != m_clips.end()) {
            m_controller->setClip(it->second);
        }
    }
}

void TimelineEdit::on_end_clicked() {
    AnimationClip *clip = m_model->clip();
    if(clip) {
        setPosition(clip->duration());
    }
}

void TimelineEdit::on_previous_clicked() {
    setPosition(m_model->findNear(position(), true));
}

void TimelineEdit::on_next_clicked() {
    setPosition(m_model->findNear(position()));
}

TString TimelineEdit::pathTo(Object *src, Object *dst) {
    TString result;
    if(src != dst) {
        TString parent = pathTo(src, dst->parent());
        if(!parent.isEmpty()) {
            result += parent + "/";
        }
        result += dst->name();
    }

    return result;
}

void TimelineEdit::on_flatKey_clicked() {
    AnimationCurve::KeyFrame *key = m_model->key(m_row, m_ind);
    if(key) {
        for(int i = 0; i < key->m_value.size(); i++) {
            m_model->commitKey(m_row, m_ind, key->m_value[i], key->m_value[i], key->m_value[i], key->m_position);
        }
    }
}

void TimelineEdit::on_breakKey_clicked() {

}

void TimelineEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void TimelineEdit::on_timeEdit_editingFinished() {
    setPosition(ui->timeEdit->text().toUInt());
}
