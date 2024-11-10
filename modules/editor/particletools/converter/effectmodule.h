#ifndef EFFECTMODULE_H
#define EFFECTMODULE_H

#include <QObject>

#include <resources/particleeffect.h>

class Widget;
class CheckBox;

class ModuleObserver;
class EffectRootNode;

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

    enum Mode {
        Constant = 0,
        Random
    };
    Q_ENUM(Mode)

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
        Renderable
    };
    Q_ENUM(Space)

    struct ParameterData {
        std::string name;

        int32_t dataType = MetaType::FLOAT;

        int mode = Constant;

        Vector4 min;
        Vector4 max;
    };

    struct OperationData {
        Operation operation;

        std::string result;

        std::string arg1;

        std::string arg2;
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

    VariantList saveData() const;

    void addParameter(const ParameterData &data);
    void addOperation(const OperationData &data);

signals:
    void updated();

    void moduleChanged();

protected:
    bool event(QEvent *e) override;

    void setMode(const ParameterData &data);

    ParameterData *parameter(const std::string &name);

    const ParameterData *parameterConst(const std::string &name) const;

protected:
    std::vector<ParameterData> m_parameters;

    std::vector<OperationData> m_operations;

    EffectRootNode *m_effect;

    Stage m_stage;

    bool m_enabled;

protected:
    CheckBox *m_checkBoxWidget;

    ModuleObserver *m_observer;

};

Q_DECLARE_METATYPE(EffectModule::Mode)

#endif // EFFECTMODULE_H
