#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectController;
class Renderable;

class QLineEdit;
class QPushButton;

class SelectTool : public EditorTool {
public:
    struct Select {
        bool operator==(const Select &left) {
            return (uuid == left.uuid);
        }

        uint32_t uuid = 0;
        Actor *object = nullptr;
    };

    typedef QList<Select> SelectList;

public:
    explicit SelectTool(ObjectController *controller);

    void beginControl() override;
    void cancelControl() override;

    QString icon() const override;
    QString name() const override;

    const VariantList &cache() const;

    QPushButton *button();

    virtual QLineEdit *snapWidget();

protected:
    Vector3 objectPosition();
    AABBox objectBound();

protected:
    VariantList m_propertiesCache;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectController *m_controller;

    QPushButton *m_button;

    QLineEdit *m_snapEditor;

};

#endif // SELECTTOOL_H
