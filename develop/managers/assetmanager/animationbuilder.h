#ifndef ANIMATIONBUILDER_H
#define ANIMATIONBUILDER_H

#include <editor/graph/abstractnodegraph.h>

#include <QVariant>

#include <editor/assetmanager.h>

class EntryState : public GraphNode {
    Q_OBJECT
public:
    Vector2 defaultSize() const override;
    Vector4 color() const override;

    bool isState() const override;

};

class BaseState : public EntryState {
    Q_OBJECT
    Q_CLASSINFO("Group", "States")

    Q_PROPERTY(QString Name READ objectName WRITE setObjectName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template Clip READ clip WRITE setClip NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool Loop READ loop WRITE setLoop NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE BaseState();

    Template clip() const;
    void setClip(const Template &path);

    bool loop() const;
    void setLoop(bool loop);

signals:
    void updated();

public:
    Template m_path;
    bool m_loop;

};

class AnimationBuilderSettings : public AssetConverterSettings {
public:
    AnimationBuilderSettings();
private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class AnimationNodeGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    AnimationNodeGraph();

    void load(const QString &path) Q_DECL_OVERRIDE;
    void save(const QString &path) Q_DECL_OVERRIDE;

    void loadGraph(const QVariantMap &data) Q_DECL_OVERRIDE;

    Variant object() const;

    QStringList nodeList() const Q_DECL_OVERRIDE;

private:
    GraphNode *createRoot() Q_DECL_OVERRIDE;
    GraphNode *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;
    Link *linkCreate(GraphNode *sender, NodePort *oport, GraphNode *receiver, NodePort *iport) Q_DECL_OVERRIDE;

    void loadUserValues(GraphNode *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(GraphNode *node, QVariantMap &values) Q_DECL_OVERRIDE;

protected:
    Variant data() const;

    GraphNode *m_entry;
    QString m_path;

    QStringList m_functions;

};

class AnimationBuilder : public AssetConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"actl"}; }

    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Animation_Controller.actl"; }

private:
    AnimationNodeGraph m_model;
};

#endif // ANIMATIONBUILDER_H
