#ifndef ANIMATIONBUILDER_H
#define ANIMATIONBUILDER_H

#include "abstractschememodel.h"

#include <QVariant>

#include "assetmanager.h"

#include <resources/animationclip.h>
#include <resources/animationstatemachine.h>

class AnimationController;

class BaseState : public QObject {
    Q_OBJECT
    Q_CLASSINFO("Group", "States")

    Q_PROPERTY(QString Name READ name WRITE setName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template Clip READ clip WRITE setClip NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool Loop READ loop WRITE setLoop NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE BaseState(AbstractSchemeModel::Node *parent) {
        m_pParent = parent;
        m_Path = Template("", MetaType::type<AnimationClip *>());
        m_Loop = false;
    }

    QString name() const {
        return m_pParent->name;
    }

    void setName(const QString &name) {
        m_pParent->name = name;
        emit updated();
    }

    Template clip() const {
        return m_Path;
    }

    void setClip(const Template &path) {
        m_Path.path = path.path;
        emit updated();
    }

    bool loop() const {
        return m_Loop;
    }

    void setLoop(bool loop) {
        m_Loop = loop;
        emit updated();
    }

signals:
    void updated();

public:
    AbstractSchemeModel::Node *m_pParent;
    Template m_Path;
    bool m_Loop;
};

class AnimationBuilder : public AbstractSchemeModel {
    Q_OBJECT

public:
    AnimationBuilder();

    QStringList suffixes() const Q_DECL_OVERRIDE { return {"actl"}; }
    uint32_t contentType() const Q_DECL_OVERRIDE { return ContentAnimationStateMachine; }
    uint32_t type() const Q_DECL_OVERRIDE { return MetaType::type<AnimationStateMachine *>(); }

    uint8_t convertFile(IConverterSettings *s) Q_DECL_OVERRIDE;

    void load(const QString &path) Q_DECL_OVERRIDE;
    void save(const QString &path) Q_DECL_OVERRIDE;

    Node *createNode(const QString &path) Q_DECL_OVERRIDE;
    void createLink(Node *sender, Item *oport, Node *receiver, Item *iport) Q_DECL_OVERRIDE;

    void loadUserValues(Node *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(Node *node, QVariantMap &values) Q_DECL_OVERRIDE;

    QAbstractItemModel *components() const Q_DECL_OVERRIDE;

protected:
    Variant object() const;

    Variant data() const;

    Node *m_pEntry;
    QString m_Path;

    QStringList m_Functions;

};

#endif // ANIMATIONBUILDER_H
