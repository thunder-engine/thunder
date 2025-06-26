#ifndef EFFECTMODULE_H
#define EFFECTMODULE_H

#include <QObject>
#include <QVariant>

#include <resources/visualeffect.h>

#include "SelectorEdit.h"

class Widget;
class CheckBox;

class EffectRootNode;

class QDomElement;

class EffectModule : public Object {
    A_OBJECT(EffectModule, Object, Modificator)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, EffectModule::enabled, EffectModule::setEnabled)
    )
    A_ENUMS(
        A_ENUM(Stage,
            A_VALUE(Spawn),
            A_VALUE(Update),
            A_VALUE(Render)
        ),
        A_ENUM(Operation,
            A_VALUE(Set),
            A_VALUE(Add),
            A_VALUE(Subtract),
            A_VALUE(Multiply),
            A_VALUE(Divide)
        ),
        A_ENUM(Space,
            A_VALUE(System),
            A_VALUE(Emitter),
            A_VALUE(Particle),
            A_VALUE(Renderable),
            A_VALUE(Local),
            A_VALUE(Constant),
            A_VALUE(Random)
        )
    )

public:
    enum Stage {
        Spawn,
        Update,
        Render
    };

    enum Operation {
        Set = 0,
        Add,
        Subtract,
        Multiply,
        Divide
    };

    enum Space {
        System,
        Emitter,
        Particle,
        Renderable,
        Local,
        Constant,
        Random
    };

    struct ParameterData {
        std::string name;

        SelectorData mode;

        Variant min;
        Variant max;

        bool visible = true;
    };

    struct OperationData {
        Operation operation;

        std::string result;

        std::vector<std::string> args;
    };

public:
    EffectModule();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    Stage stage() const { return m_stage; }
    void setStage(Stage stage) { m_stage = stage; }

    void setRoot(EffectRootNode *effect);

    Widget *widget(Object *parent);

    void load(const std::string &path);
    void fromXml(const QDomElement &element);

    VariantList saveData() const override;

    void addParameter(const ParameterData &data);
    void addOperation(const OperationData &data);

    ParameterData *parameter(const std::string &name);

protected:
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

};

#endif // EFFECTMODULE_H
