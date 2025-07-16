#ifndef UIDOCUMENT_H
#define UIDOCUMENT_H

#include <resource.h>
#include <uikit.h>

class UIKIT_EXPORT UiDocument : public Resource {
    A_OBJECT(UiDocument, Resource, Resources)

public:
    UiDocument();

    String data() const;
    void setData(const String &data);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    String m_data;

};

#endif // UIDOCUMENT_H
