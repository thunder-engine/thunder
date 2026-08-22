#ifndef ABSTRACTITEMMODEL_H
#define ABSTRACTITEMMODEL_H

#include <variant.h>

class NEXT_LIBRARY_EXPORT AbstractItemModel {
public:
    enum Roles {
        DisplayRole	= 0,
        DecorationRole,
        EditRole,
        ToolTipRole
    };

public:
    virtual ~AbstractItemModel() {};

    virtual int rowCount(const ModelIndex &parent = ModelIndex()) const = 0;
    virtual int columnCount(const ModelIndex &parent = ModelIndex()) const = 0;

    virtual ModelIndex index(int row, int column, const ModelIndex &parent = ModelIndex()) const = 0;
    virtual ModelIndex parent(const ModelIndex &index) const = 0;

    virtual Variant data(const ModelIndex &index, int role = 0) const = 0;

protected:
    ModelIndex createIndex(int row, int column, uint32_t uuid) const;

};

#endif // ABSTRACTITEMMODEL_H
