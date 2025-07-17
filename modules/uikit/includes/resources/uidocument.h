#ifndef UIDOCUMENT_H
#define UIDOCUMENT_H

#include <resource.h>
#include <uikit.h>

class UIKIT_EXPORT UiDocument : public Resource {
    A_OBJECT(UiDocument, Resource, Resources)

public:
    UiDocument();

    TString data() const;
    void setData(const TString &data);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    TString m_data;

};

#endif // UIDOCUMENT_H
