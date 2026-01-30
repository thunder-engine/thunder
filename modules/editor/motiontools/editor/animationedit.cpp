#include "animationedit.h"
#include "ui_animationedit.h"

#include <components/world.h>
#include <components/animator.h>
#include <resources/animationstatemachine.h>

#include <editor/undostack.h>

#include <QToolButton>

#include "renamedialog.h"
#include "actions/addvariable.h"
#include "actions/removevariable.h"
#include "actions/renamevariable.h"
#include "../converter/animationbuilder.h"

#include "statelink.h"

namespace {
    const char *gProperty("property");
};

class AnimationProxy : public Object {
    A_OBJECT(AnimationProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(AnimationProxy::onGraphUpdated),
        A_SLOT(AnimationProxy::onVariableChanged)
    )
public:
    void setEditor(AnimationEdit *editor) {
        m_editor = editor;
    }

    void onGraphUpdated() {
        m_editor->updated();
    }

    void onVariableChanged() {
        m_editor->onActivated();
    }

private:
    AnimationEdit *m_editor = nullptr;

};

AnimationEdit::AnimationEdit() :
        ui(new Ui::AnimationEdit),
        m_graph(new AnimationControllerGraph),
        m_assetConverter(new AnimationControllerBuilder),
        m_stateMachine(nullptr),
        m_proxy(new AnimationProxy),
        m_variableButton(nullptr) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    Object::connect(m_graph, _SIGNAL(graphUpdated()), m_proxy, _SLOT(onGraphUpdated()));
    Object::connect(m_graph, _SIGNAL(variableChanged()), m_proxy, _SLOT(onVariableChanged()));

    connect(ui->schemeWidget, &GraphView::objectsSelected, this, &AnimationEdit::onObjectsSelected);

    ui->schemeWidget->setEditor(this);
    ui->schemeWidget->setWorld(Engine::objectCreate<World>("World"));
    ui->schemeWidget->setGraph(m_graph);
    ui->schemeWidget->init();

    QAction *renAction = m_variablesMenu.addAction(tr("Rename Variable"));
    connect(renAction, &QAction::triggered, this, &AnimationEdit::onRenameVariable);

    QAction *delAction = m_variablesMenu.addAction(tr("Delete Variable"));
    connect(delAction, &QAction::triggered, this, &AnimationEdit::onDeleteVariable);
}

AnimationEdit::~AnimationEdit() {
    delete ui;
}

bool AnimationEdit::isModified() const {
    return !m_undoRedo->isClean();
}

StringList AnimationEdit::suffixes() const {
    StringList result;
    for(auto &it : m_assetConverter->suffixes()) {
        result.push_back(it);
    }
    return result;
}

void AnimationEdit::onAddVariable(QAction *action) {
    TString name;
    Variant value;
    switch(action->data().toInt()) {
        case MetaType::BOOLEAN: name = "New Bool"; value = false; break;
        case MetaType::INTEGER: name = "New Int"; value = 0; break;
        case MetaType::FLOAT: name = "New Float"; value = 0.0f; break;
        default: break;
    }

    m_undoRedo->push(new AddVariable(name, value, m_graph));
}

void AnimationEdit::onActivated() {
    ui->schemeWidget->reselect();
}

void AnimationEdit::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    ui->schemeWidget->onObjectsChanged(objects, property, value);
}

void AnimationEdit::onCutAction() {
    ui->schemeWidget->onCutAction();
}

void AnimationEdit::onCopyAction() {
    ui->schemeWidget->onCopyAction();
}

void AnimationEdit::onPasteAction() {
    ui->schemeWidget->onPasteAction();
}

QWidget *AnimationEdit::propertiesWidget() {
    if(m_variableButton == nullptr) {
        m_variableButton = new QToolButton;

        m_variableButton->setProperty("blue", true);
        m_variableButton->setPopupMode(QToolButton::InstantPopup);
        m_variableButton->setText(tr("Add Variable"));
        m_variableButton->setToolTip(tr("Adds a new Animator state machine variable."));
        m_variableButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_variableButton->setMinimumHeight(25);
        m_variableButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        QMenu *rootMenu = new QMenu;
        rootMenu->addAction("Boolean")->setData(MetaType::BOOLEAN);
        rootMenu->addAction("Integer")->setData(MetaType::INTEGER);
        rootMenu->addAction("Float")->setData(MetaType::FLOAT);

        connect(rootMenu, &QMenu::triggered, this, &AnimationEdit::onAddVariable);

        m_variableButton->setMenu(rootMenu);
    }

    return m_variableButton;
}

QMenu *AnimationEdit::propertyContextMenu(Object *object, const TString &property) {
    if(object == m_graph) {
        m_variablesMenu.setProperty(gProperty, property.data());
        return &m_variablesMenu;
    }
    return nullptr;
}

void AnimationEdit::onObjectsSelected(const Object::ObjectList &objects) {
    if(!objects.empty()) {
        if(m_variableButton) {
            m_variableButton->setVisible(objects.front() == m_graph);
        }

        StateLink *state = dynamic_cast<StateLink *>(objects.front());
        if(state) {
            Object::connect(state, _SIGNAL(variableChanged()), m_proxy, _SLOT(onVariableChanged()));
        }
    }

    emit objectsSelected(objects);
}

void AnimationEdit::onRenameVariable() {
    QString oldName = m_variablesMenu.property(gProperty).toString();
    RenameDialog dlg;
    dlg.setText(oldName);

    if(dlg.exec() == QDialog::Accepted) {
        m_undoRedo->push(new RenameVariable(qPrintable(oldName), qPrintable(dlg.text()), m_graph));
    }
}

void AnimationEdit::onDeleteVariable() {
    m_undoRedo->push(new RemoveVariable(qPrintable(m_variablesMenu.property(gProperty).toString()), m_graph));
}

bool AnimationEdit::isCopyActionAvailable() const {
    return ui->schemeWidget->isCopyActionAvailable();
}

bool AnimationEdit::isPasteActionAvailable() const {
    return ui->schemeWidget->isPasteActionAvailable();
}

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
        AssetEditor::loadAsset(settings);

        m_stateMachine = Engine::loadResource<AnimationStateMachine>(settings->destination());

        m_graph->load(settings->source());
    }
}

void AnimationEdit::saveAsset(const TString &path) {
    if(!path.isEmpty() || !m_settings.front()->source().isEmpty()) {
        m_graph->save(path.isEmpty() ? m_settings.front()->source() : path);
    }
}

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
