#include "tst_common.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/component.h"
#include "components/camera.h"
#include "components/meshrender.h"
#include "components/skinnedmeshrender.h"

#include "resources/prefab.h"

#include "systems/resourcesystem.h"
#include "systems/rendersystem.h"

#include "commandbuffer.h"

#include <json.h>

class TestComponent : public Component {
public:
    A_REGISTER(TestComponent, Component, Components);

    A_PROPERTIES (
        A_PROPERTYEX(TestComponent *, reference, TestComponent::reference, TestComponent::setReference, "editor=Component")
    )
    A_NOMETHODS()

    TestComponent() {
        m_Reference = nullptr;
    }

    TestComponent *reference() const {
        return m_Reference;
    }

    void setReference(TestComponent *value) {
        m_Reference = value;
    }

    TestComponent *m_Reference;

};

class ActorTest : public QObject {
    Q_OBJECT
private slots:

void Basic_properties() {
    Actor actor;

    QCOMPARE(actor.isEnabled(), true);
    actor.setEnabled(false);
    QCOMPARE(actor.isEnabled(), false);

    QCOMPARE(actor.layers(), CommandBuffer::DEFAULT | CommandBuffer::RAYCAST | CommandBuffer::SHADOWCAST| CommandBuffer::TRANSLUCENT);
    actor.setLayers(CommandBuffer::UI);
    QCOMPARE(actor.layers(), CommandBuffer::UI);
}

void Transform_hierarchy() {
    ObjectSystem system;
    Actor::registerClassFactory(&system);
    Transform::registerClassFactory(&system);

    Actor a1;
    a1.addComponent("Transform");

    Actor a2;
    a2.addComponent("Transform");

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
    SkinnedMeshRender::registerClassFactory(&system);
    MeshRender::registerClassFactory(&system);
    RenderSystem render;

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    prefab->addComponent("Transform");
    QCOMPARE(prefab != nullptr, true);

    Transform *t0 = prefab->transform();
    QCOMPARE(t0 != nullptr, true);
    t0->setPosition(Vector3(1.0f, 2.0f, 3.0f));

    SkinnedMeshRender *prefabSkinned = dynamic_cast<SkinnedMeshRender *>(prefab->addComponent("SkinnedMeshRender"));
    QCOMPARE(prefabSkinned != nullptr, true);

    Actor *level1 = Engine::composeActor("", "Level1", prefab);
    Camera *prefabCamera = dynamic_cast<Camera *>(level1->addComponent("Camera"));
    QCOMPARE(prefabCamera != nullptr, true);
    prefabCamera->setFocal(10.0f);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

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

    Actor *level2 = Engine::composeActor("", "Level2", cloneCamera->actor());
    MeshRender *cloneMesh = dynamic_cast<MeshRender *>(level2->addComponent("MeshRender"));
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
    QCOMPARE(resultMesh->actor()->name(), "Level2");

    delete result;

    delete clone;
    delete prefab;
}

void Cross_reference_prefab() {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    QCOMPARE(prefab != nullptr, true);

    Actor *level1 = Engine::objectCreate<Actor>("Level1", prefab);
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(level1->addComponent("TestComponent"));
    QCOMPARE(prefabTestComponent != nullptr, true);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    Actor *root = Engine::objectCreate<Actor>("Root");

    Actor *clone1 = dynamic_cast<Actor *>(prefab->clone(root));
    QCOMPARE(clone1 != nullptr, true);

    TestComponent *cloneTestComponent1 = dynamic_cast<TestComponent *>(clone1->componentInChild("TestComponent"));
    QCOMPARE(cloneTestComponent1 != nullptr, true);

    Actor *clone2 = dynamic_cast<Actor *>(prefab->clone(root));
    QCOMPARE(clone2 != nullptr, true);

    TestComponent *cloneTestComponent2 = dynamic_cast<TestComponent *>(clone2->componentInChild("TestComponent"));
    QCOMPARE(cloneTestComponent2 != nullptr, true);

    cloneTestComponent1->setReference(cloneTestComponent2);
    cloneTestComponent2->setReference(cloneTestComponent1);

    Variant data = Engine::toVariant(root);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    QCOMPARE(result != nullptr, true);

    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(result->componentInChild("TestComponent"));
    QCOMPARE(resultTestComponent != nullptr, true);
    TestComponent *referenceTestComponent = resultTestComponent->reference();

    QCOMPARE(referenceTestComponent != nullptr, true);

    QCOMPARE(referenceTestComponent != resultTestComponent, true);
    QCOMPARE(resultTestComponent->reference() == referenceTestComponent, true);
    QSKIP("Temporary disabled");
    QCOMPARE(referenceTestComponent->reference() == resultTestComponent, true);

    delete result;

    delete clone2;
    delete clone1;
    delete prefab;
}

void Remove_component_from_prefab_instance() {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    QCOMPARE(prefab != nullptr, true);

    Actor *level1 = Engine::objectCreate<Actor>("Level1", prefab);
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(level1->addComponent("TestComponent"));
    QCOMPARE(prefabTestComponent != nullptr, true);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    QCOMPARE(clone != nullptr, true);
    QCOMPARE(clone->isInstance(), true);

    TestComponent *cloneTestComponent = dynamic_cast<TestComponent *>(clone->componentInChild("TestComponent"));
    QCOMPARE(cloneTestComponent != nullptr, true);

    delete cloneTestComponent;

    Variant data = Engine::toVariant(clone);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    QCOMPARE(result != nullptr, true);

    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(result->componentInChild("TestComponent"));
    QCOMPARE(resultTestComponent == nullptr, true);

    delete result;

    delete clone;
    delete prefab;
}

void Update_prefab_instance() {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    QCOMPARE(prefab != nullptr, true);

    Actor *level1 = Engine::composeActor("TestComponent", "Level1", prefab);

    uint32_t uuidLevel1 = level1->uuid();

    Actor *level2 = Engine::composeActor("", "Level2", prefab);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    // Create an instance before do changes in prefab
    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    QCOMPARE(clone != nullptr, true);
    QCOMPARE(clone->isInstance(), true);

    fab->switchState(Resource::Loading);

    // Step 1 - Add item to prefab
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(prefab->addComponent("TestComponent"));
    QCOMPARE(prefabTestComponent != nullptr, true);

    // Step 2 - Delete item from prefab
    delete level1;

    // Step 3 - Change prefab property
    const Vector3 value(1.0f, 2.0f, 3.0f);
    level2->transform()->setPosition(value);

    // Sync instance with prefab
    fab->switchState(Resource::Ready);

    // Check instance state from Step 1
    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(clone->component("TestComponent"));
    QCOMPARE(resultTestComponent != nullptr, true);
    QCOMPARE(QString(resultTestComponent->parent()->name().c_str()), "Prefab");

    // Check instance state from Step 2
    Actor *cloneLevel1 = dynamic_cast<Actor *>(Engine::findObject(uuidLevel1, clone));
    QCOMPARE(cloneLevel1 == nullptr, true);

    // Check instance state from Step 3
    Actor *cloneLevel2 = dynamic_cast<Actor *>(Engine::findObject(level2->uuid(), clone));
    QCOMPARE(cloneLevel2 != nullptr, true);
    QCOMPARE(QString(cloneLevel2->name().c_str()), QString(level2->name().c_str()));
    QCOMPARE(cloneLevel2->transform() != nullptr, true);
    QCOMPARE(cloneLevel2->transform()->position(), value);

    delete prefab;
}

} REGISTER(ActorTest)

#include "tst_actor.moc"
