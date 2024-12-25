#ifndef EFFECTMODULE_H
#define EFFECTMODULE_H

#include <QObject>
#include <QVariant>

#include <resources/visualeffect.h>

#include "SelectorEdit.h"

class Widget;
class CheckBox;

class ModuleObserver;
class EffectRootNode;

class QDomElement;

class EffectModule : public QObject {
    Q_OBJECT
    Q_CLASSINFO("Group", "Modificator")

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY updated DESIGNABLE true USER true)

public:
    enum Stage {
        Spawn,
        Update,
        Render
    };
    Q_ENUM(Stage);

    enum Operation {
        Set = 0,
        Add,
        Subtract,
        Multiply,
        Divide
    };
    Q_ENUM(Operation)

    enum Space {
        System,
        Emitter,
        Particle,
        Renderable,
        Local,
        Constant,
        Random
    };
    Q_ENUM(Space)

    struct ParameterData {
        std::string name;

        SelectorData mode;

        QVariant min;
        QVariant max;

        bool visible = true;
    };

    struct OperationData {
        Operation operation;

        std::string result;

        std::vector<std::string> args;
    };

public:
    Q_INVOKABLE EffectModule();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    Stage stage() const { return m_stage; }
    void setStage(Stage stage) { m_stage = stage; }

    void setRoot(EffectRootNode *effect);

    Widget *widget(Object *parent);

    void load(const std::string &path);
    void fromXml(const QDomElement &element);

    VariantList saveData() const;

    void addParameter(const ParameterData &data);
    void addOperation(const OperationData &data);

    ParameterData *parameter(const std::string &name);

signals:
    void updated();

    void moduleChanged();

protected:
    bool event(QEvent *e) override;

    const ParameterData *parameterConst(const std::string &name) const;

protected:
    std::vector<ParameterData> m_parameters;

    std::vector<OperationData> m_operations;

    std::map<std::string, std::vector<std::string>> m_options;

    EffectRootNode *m_effect;

    Stage m_stage;

    bool m_enabled;

protected:
    CheckBox *m_checkBoxWidget;

    ModuleObserver *m_observer;

};

#endif // EFFECTMODULE_H
