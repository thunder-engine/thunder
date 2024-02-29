#ifndef UIDOCUMENT_H
#define UIDOCUMENT_H

#include <resource.h>
#include <uikit.h>

class UIKIT_EXPORT UiDocument: public Resource {
    A_REGISTER(UiDocument, Resource, Resources)

public:
    UiDocument();

    string data() const;
    void setData(const string &data);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const  override;

private:
    string m_data;

};

#endif // UIDOCUMENT_H
