#ifndef CUSTOMMODULE_H
#define CUSTOMMODULE_H

#include "effectmodule.h"

class CustomModule : public EffectModule {
    A_OBJECT(CustomModule, EffectModule, Modificator)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TString path() const;

    void load(const TString &path);

    TString typeName() const override;

private:
    void getOperations(std::vector<OperationData> &operations) const override;

protected:
    TString m_path;

    struct OperationStr {
        Operation operation;

        TString result;

        std::vector<TString> arguments;
    };

    std::vector<OperationStr> m_operations;

};

#endif // CUSTOMMODULE_H
