#ifndef CONTROLSCHEME_H
#define CONTROLSCHEME_H

#include <resource.h>

class ENGINE_EXPORT ControlScheme : public Resource {
    A_OBJECT(ControlScheme, Resource, Resources)

public:
    int actionsCount() const;
    TString actionName(int action) const;

    int bindingsCount(int action) const;
    int bindingCode(int action, int binding);
    bool bindingNegative(int action, int binding);

    void loadUserData(const VariantMap &data) override;

protected:
    VariantMap saveUserData() const override;

private:
    struct Binding {
        TString path;
        int32_t code;
        bool negative;
    };

    struct Action {
        TString name;
        std::vector<Binding> bindings;
    };

    std::vector<Action> m_actions;

};

#endif // CONTROLSCHEME_H
