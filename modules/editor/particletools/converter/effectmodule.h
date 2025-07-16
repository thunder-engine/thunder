#ifndef EFFECTMODULE_H
#define EFFECTMODULE_H

#include <QObject>
#include <QVariant>

#include <resources/visualeffect.h>

class Widget;
class CheckBox;

class EffectRootNode;

class QDomElement;
class QDomDocument;

class EffectModule : public Object {
    A_OBJECT(EffectModule, Object, Modificator)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, EffectModule::enabled, EffectModule::setEnabled)
    )
    A_METHODS(
        A_SLOT(EffectModule::setEnabled)
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
            A_VALUE(_System),
            A_VALUE(_Emitter),
            A_VALUE(_Particle),
            A_VALUE(_Renderable),
            A_VALUE(_Local),
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
        None = -1,
        _System,
        _Emitter,
        _Particle,
        _Renderable,
        _Local,
        Constant,
        Random
    };

    struct ParameterData {
        String name;
        String type;
        String modeType;

        Variant min;
        Variant max;

        int mode;

        bool visible = true;
    };

    struct OperationData {
        Operation operation;

        String result;

        std::vector<String> args;
    };

public:
    EffectModule();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    Stage stage() const { return m_stage; }
    void setStage(Stage stage) { m_stage = stage; }

    void setRoot(EffectRootNode *effect);

    Widget *widget(Object *parent);

    String path() const;

    void load(const String &path);

    void toXml(QDomElement &element, QDomDocument &xml);
    void fromXml(const QDomElement &element);

    VariantList saveData() const override;

    ParameterData *parameter(const String &name);

protected:
    const ParameterData *parameterConst(const String &name) const;

    const char *annotationHelper(const String &type) const;

    void setProperty(const char *name, const Variant &value) override;

protected:
    std::vector<ParameterData> m_parameters;

    std::vector<OperationData> m_operations;

    String m_path;

    EffectRootNode *m_effect;

    Stage m_stage;

    bool m_enabled;
    bool m_blockUpdate;

protected:
    CheckBox *m_checkBoxWidget;

};

#endif // EFFECTMODULE_H
