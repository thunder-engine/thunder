#include "tst_common.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/component.h"
#include "components/camera.h"
#include "components/meshrender.h"
#include "components/skinnedmeshrender.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"

#include <json.h>

class ActorTest : public QObject {
    Q_OBJECT
private slots:

void Basic_properties() {
    Actor actor;

    QCOMPARE(actor.isEnabled(), true);
    actor.setEnabled(false);
    QCOMPARE(actor.isEnabled(), false);

    QCOMPARE(actor.layers(), ICommandBuffer::DEFAULT | ICommandBuffer::RAYCAST | ICommandBuffer::SHADOWCAST| ICommandBuffer::TRANSLUCENT);
    actor.setLayers(ICommandBuffer::UI);
    QCOMPARE(actor.layers(), ICommandBuffer::UI);
}

void Transform_hierarchy() {
    ObjectSystem system;
    Actor::registerClassFactory(&system);
    Transform::registerClassFactory(&system);

    Actor a1;
    Actor a2;

    Transform *t1 = a1.transform();
    Transform *t2 = a2.transform();

    QCOMPARE(t1 != nullptr, true);
    QCOMPARE(t2 != nullptr, true);

    a2.setParent(&a1);

    QCOMPARE(t2->parentTransform() == t1, true);
}

void Add_Remove_Component() {
    ObjectSystem system;
    Actor::registerClassFactory(&system);
    Component::registerClassFactory(&system);

    Actor a1;

    Component *component = a1.addComponent("Component");
    QCOMPARE(a1.getChildren().size(), 1);

    Component *result1 = a1.component("Component");
    QCOMPARE(component == result1, true);

    Actor parent;
    a1.setParent(&parent);

    Component *result2 = parent.componentInChild("Component");
    QCOMPARE(component == result2, true);

    delete component;
    QCOMPARE(a1.getChildren().size(), 0);
}

void Prefab_serialization() {
    Engine system(nullptr, "");

    Actor *prefab = Engine::objectCreate<Actor>("");
    QCOMPARE(prefab != nullptr, true);

    Transform *t0 = prefab->transform();
    QCOMPARE(t0 != nullptr, true);
    t0->setPosition(Vector3(1.0f, 2.0f, 3.0f));

    SkinnedMeshRender *prefabSkinned = dynamic_cast<SkinnedMeshRender *>(prefab->addComponent("SkinnedMeshRender"));
    QCOMPARE(prefabSkinned != nullptr, true);

    Actor *level1 = Engine::objectCreate<Actor>("Level1", prefab);
    Camera *prefabCamera = dynamic_cast<Camera *>(level1->addComponent("Camera"));
    QCOMPARE(prefabCamera != nullptr, true);
    prefabCamera->setFocal(10.0f);

    static_cast<ResourceSystem *>(system.resourceSystem())->setResource(prefab, "TestPrefab");

    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    QCOMPARE(clone != nullptr, true);

    QCOMPARE(clone->transform()->position() == t0->position(), true);
    Camera *cloneCamera = dynamic_cast<Camera *>(clone->componentInChild("Camera"));
    QCOMPARE(cloneCamera != nullptr, true);
    QCOMPARE(cloneCamera->focal(), 10.0f);
    QCOMPARE(cloneCamera->actor()->name(), "Level1");

    Transform *t1 = cloneCamera->actor()->transform();
    QCOMPARE(t1 != nullptr, true);
    t1->setPosition(Vector3(3.0f, 2.0f, 1.0f));

    MeshRender *cloneMesh = dynamic_cast<MeshRender *>(cloneCamera->actor()->addComponent("MeshRender"));
    QCOMPARE(cloneMesh != nullptr, true);

    Variant data = Engine::toVariant(clone);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    QCOMPARE(result != nullptr, true);

    QCOMPARE(result->transform()->position() == t0->position(), true);
    Camera *resultCamera = dynamic_cast<Camera *>(result->componentInChild("Camera"));
    QCOMPARE(resultCamera != nullptr, true);
    QCOMPARE(resultCamera->focal(), 10.0f);
    QCOMPARE(resultCamera->actor()->name(), "Level1");
    QCOMPARE(resultCamera->actor()->transform()->position() == t1->position(), true);

    MeshRender *resultMesh = dynamic_cast<MeshRender *>(result->componentInChild("MeshRender"));
    QCOMPARE(resultMesh != nullptr, true);
    QCOMPARE(resultMesh->actor()->name(), "Level1");

    delete result;

    delete clone;
    delete prefab;
}

} REGISTER(ActorTest)

#include "tst_actor.moc"
