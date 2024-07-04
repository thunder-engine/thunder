#include "timelineedit.h"
#include "ui_timelineedit.h"

#include <json.h>

#include <QFileInfo>

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
        m_controller(nullptr),
        m_armature(nullptr),
        m_model(new AnimationClipModel(this)),
        m_lastCommand(nullptr),
        m_timerId(0),
        m_row(-1),
        m_col(-1),
        m_ind(-1) {

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

    connect(ui->clipBox, SIGNAL(activated(QString)), this, SLOT(onClipChanged(QString)));

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
    const UndoCommand *lastCommand = UndoManager::instance()->lastCommand(m_model);
    AnimationClip *clip = m_model->clip();
    if(m_lastCommand != lastCommand && clip) {
        VariantMap data = clip->saveUserData();

        std::string ref = Engine::reference(clip);
        QFile file(AssetManager::instance()->guidToPath(ref).c_str());
        if(file.open(QIODevice::WriteOnly)) {
            file.write(Json::save(data["Tracks"], 0).c_str());
            file.close();

            m_lastCommand = lastCommand;
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

void TimelineEdit::onObjectsSelected(QList<Object *> objects) {
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

void TimelineEdit::onItemsSelected(QList<QObject *> objects) {

}

void TimelineEdit::updateClips() {
    m_clips.clear();

    if(m_controller) {
        AnimationStateMachine *stateMachine = m_controller->stateMachine();
        if(stateMachine) {
            for(auto it : stateMachine->states()) {
                std::string ref = Engine::reference(it->m_clip);
                QFileInfo info(AssetManager::instance()->guidToPath(ref).c_str());
                m_clips[info.baseName()] = it->m_clip;
            }
            if(!m_clips.isEmpty()) {
                m_currentClip = m_clips.begin().key();
                m_model->setClip(m_clips.begin().value(), m_controller->actor());
                m_controller->setClip(m_clips.begin().value());
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
    ui->clipBox->addItems(m_clips.keys());

    onSelectKey(-1, -1, -1);

    on_begin_clicked();
}

uint32_t TimelineEdit::position() const {
    if(m_controller) {
        return m_controller->position();
    }
    return 0;
}

void TimelineEdit::setPosition(uint32_t position) {
    if(m_controller) {
        m_controller->setPosition(position);
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

void TimelineEdit::onObjectsChanged(QList<Object *> objects, const QString property, Variant value) {
    for(auto it : objects) {
        onPropertyUpdated(it, property);
    }
}

void TimelineEdit::onPropertyUpdated(Object *object, const QString property) {
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

void TimelineEdit::onRebind() {
    m_controller->rebind();
}

void TimelineEdit::onSelectKey(int row, int col, int index) {
    m_row = row;
    m_col = col;
    m_ind = index;

    ui->valueEdit->clear();
    ui->timeEdit->clear();

    if(row > -1 && col == -1) {
        col = 0;
    }
    AnimationCurve::KeyFrame *key = m_model->key(row, col, index);
    if(key) {
        AnimationTrack &t = m_model->track(row);
        if(m_col > -1) {
            ui->valueEdit->setText(QString::number(key->m_Value));
        }
        ui->timeEdit->setText(QString::number(key->m_Position * t.duration()));
    }
    ui->deleteKey->setEnabled(key != nullptr);
    ui->flatKey->setEnabled(key != nullptr);
}

void TimelineEdit::onRowsSelected(QStringList list) {
    QList<Object *> result;
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
        emit objectsSelected(result, false);
    }
}

void TimelineEdit::onClipChanged(const QString &clip) {
    m_currentClip = clip;
    m_model->setClip(m_clips.value(m_currentClip), m_controller->actor());
    m_controller->setClip(m_clips.value(m_currentClip));
}

void TimelineEdit::onKeyChanged() {
    AnimationCurve::KeyFrame *key = m_model->key(m_row, m_col, m_ind);
    if(key) {
        float delta = ui->valueEdit->text().toFloat() - key->m_Value;
        m_model->commitKey(m_row, m_col, m_ind, key->m_Value + delta,
                                                 key->m_LeftTangent + delta,
                                                 key->m_RightTangent + delta, ui->timeEdit->text().toUInt());
    }
}

void TimelineEdit::timerEvent(QTimerEvent *) {
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

void TimelineEdit::on_play_clicked() {
    if(m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    } else {
        m_timerId = startTimer(16);
    }
}

void TimelineEdit::on_begin_clicked() {
    setPosition(0);
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

QString TimelineEdit::pathTo(Object *src, Object *dst) {
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

void TimelineEdit::on_flatKey_clicked() {
    AnimationCurve::KeyFrame *key = m_model->key(m_row, m_col, m_ind);
    if(key) {
        m_model->commitKey(m_row, m_col, m_ind, key->m_Value, key->m_Value, key->m_Value, key->m_Position);
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
