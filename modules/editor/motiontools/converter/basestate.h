#ifndef BASESTATE_H
#define BASESTATE_H

#include "entrystate.h"

#include <editor/assetmanager.h>

class BaseState : public EntryState {
    Q_OBJECT
    Q_CLASSINFO("Group", "States")

    Q_PROPERTY(QString Name READ objectName WRITE setObjectName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template Clip READ clip WRITE setClip NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool Loop READ loop WRITE setLoop NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE BaseState();

    QString name() const;
    void setName(const QString &name);

    Template clip() const;
    void setClip(const Template &path);

    bool loop() const;
    void setLoop(bool loop);

    Vector4 color() const override;

public:
    Template m_path;

    bool m_loop;

};

#endif // BASESTATE_H
