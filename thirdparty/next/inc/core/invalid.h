#ifndef INVALID_H
#define INVALID_H

#include "object.h"

class NEXT_LIBRARY_EXPORT Invalid : public Object {
public:
    Invalid();

    void loadData(const VariantList &data);

    VariantList saveData() const;

    string typeName() const;

protected:
    VariantList m_data;

};

#endif // INVALID_H
