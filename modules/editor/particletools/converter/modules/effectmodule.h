#ifndef EFFECTMODULE_H
#define EFFECTMODULE_H

#include <resources/visualeffect.h>

class Widget;
class CheckBox;

class EffectRootNode;

namespace pugi {
    class xml_document;
    class xml_node;
}

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
            A_VALUE(EmitterSpawn),
            A_VALUE(EmitterUpdate),
            A_VALUE(ParticleSpawn),
            A_VALUE(ParticleUpdate),
            A_VALUE(Render)
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
        EmitterSpawn,
        EmitterUpdate,
        ParticleSpawn,
        ParticleUpdate,
        Render
    };

    enum Operation {
        Set = 0,
        Add,
        Subtract,
        Multiply,
        Divide,
        Mod,
        Min,
        Max,
        Floor,
        Ceil,
        Make
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

    struct VariableData {
        Space space = Space::_Particle;
        int32_t offset = 0;
        int32_t size = 0;

        Vector4 min;
        Vector4 max;
    };

    struct OperationData {
        Operation operation;

        VariableData result;

        std::vector<VariableData> arguments;
    };

public:
    EffectModule();

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    Stage stage() const { return m_stage; }

    virtual void setRoot(EffectRootNode *effect);

    Widget *widget(Object *parent);

    void toXml(pugi::xml_node &element);
    void fromXml(const pugi::xml_node &element);

    VariantList saveData() const override;

    VariableData variable(const TString &name) const;

    static int typeSize(uint32_t type);

    static MetaType::Type type(const TString &name);

protected:
    const char *annotationHelper(const TString &type) const;

    void setProperty(const char *name, const Variant &value) override;

    virtual void getOperations(std::vector<OperationData> &operations) const;

protected:
    const std::map<TString, int> m_locals;

    EffectRootNode *m_effect;

    Stage m_stage;

    bool m_enabled;
    bool m_blockUpdate;

protected:
    CheckBox *m_checkBoxWidget;

};

#endif // EFFECTMODULE_H
