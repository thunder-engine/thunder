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

class ActorTest : public ::testing::Test {
public:
    void switchStatePrefab(Prefab &fab, ResourceState state) {
        fab.switchState(state);
    }
};

TEST_F(ActorTest, Basic_properties) {
    Actor actor;

    ASSERT_TRUE(actor.isEnabled());
    actor.setEnabled(false);
    ASSERT_FALSE(actor.isEnabled());

    ASSERT_TRUE(actor.layers() == (CommandBuffer::DEFAULT | CommandBuffer::RAYCAST | CommandBuffer::SHADOWCAST| CommandBuffer::TRANSLUCENT));
    actor.setLayers(CommandBuffer::UI);
    ASSERT_TRUE(actor.layers() == CommandBuffer::UI);
}

TEST_F(ActorTest, Transform_hierarchy) {
    ObjectSystem system;
    Actor::registerClassFactory(&system);
    Transform::registerClassFactory(&system);

    Actor a1;
    a1.addComponent("Transform");

    Actor a2;
    a2.addComponent("Transform");

    Transform *t1 = a1.transform();
    Transform *t2 = a2.transform();

    ASSERT_TRUE(t1 != nullptr);
    ASSERT_TRUE(t2 != nullptr);

    a2.setParent(&a1);

    ASSERT_TRUE(t2->parentTransform() == t1);
}

TEST_F(ActorTest, Add_Remove_Component) {
    ObjectSystem system;
    Actor::registerClassFactory(&system);
    Component::registerClassFactory(&system);

    Actor parent;

    Actor a1;

    Component *component = a1.addComponent("Component");
    ASSERT_TRUE(a1.getChildren().size() == 1);

    Component *result1 = a1.component("Component");
    ASSERT_TRUE(component == result1);

    a1.setParent(&parent);

    Component *result2 = parent.componentInChild("Component");
    ASSERT_TRUE(component == result2);

    delete component;
    ASSERT_TRUE(a1.getChildren().size() == 0);
}

TEST_F(ActorTest, Prefab_serialization) {
    Engine system(nullptr, "");
    SkinnedMeshRender::registerClassFactory(&system);
    MeshRender::registerClassFactory(&system);
    RenderSystem render;

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    prefab->addComponent("Transform");
    ASSERT_TRUE(prefab != nullptr);

    Transform *t0 = prefab->transform();
    ASSERT_TRUE(t0 != nullptr);
    t0->setPosition(Vector3(1.0f, 2.0f, 3.0f));

    SkinnedMeshRender *prefabSkinned = dynamic_cast<SkinnedMeshRender *>(prefab->addComponent("SkinnedMeshRender"));
    ASSERT_TRUE(prefabSkinned != nullptr);

    Actor *level1 = Engine::composeActor("", "Level1", prefab);
    Camera *prefabCamera = dynamic_cast<Camera *>(level1->addComponent("Camera"));
    ASSERT_TRUE(prefabCamera != nullptr);
    prefabCamera->setFocal(10.0f);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    ASSERT_TRUE(clone != nullptr);

    ASSERT_TRUE(clone->transform()->position() == t0->position());
    Camera *cloneCamera = dynamic_cast<Camera *>(clone->componentInChild("Camera"));
    ASSERT_TRUE(cloneCamera != nullptr);
    ASSERT_TRUE(cloneCamera->focal() == 10.0f);
    ASSERT_TRUE(cloneCamera->actor()->name() == "Level1");

    Transform *t1 = cloneCamera->transform();
    ASSERT_TRUE(t1 != nullptr);
    t1->setPosition(Vector3(3.0f, 2.0f, 1.0f));

    Actor *level2 = Engine::composeActor("", "Level2", cloneCamera->actor());
    MeshRender *cloneMesh = dynamic_cast<MeshRender *>(level2->addComponent("MeshRender"));
    ASSERT_TRUE(cloneMesh != nullptr);

    Variant data = Engine::toVariant(clone);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    ASSERT_TRUE(result != nullptr);

    ASSERT_TRUE(result->transform()->position() == t0->position());
    Camera *resultCamera = dynamic_cast<Camera *>(result->componentInChild("Camera"));
    ASSERT_TRUE(resultCamera != nullptr);
    ASSERT_TRUE(resultCamera->focal() == 10.0f);
    ASSERT_TRUE(resultCamera->actor()->name() == "Level1");
    ASSERT_TRUE(resultCamera->transform()->position() == t1->position());

    MeshRender *resultMesh = dynamic_cast<MeshRender *>(result->componentInChild("MeshRender"));
    ASSERT_TRUE(resultMesh != nullptr);
    ASSERT_TRUE(resultMesh->actor()->name() == "Level2");

    delete result;

    delete clone;
    delete prefab;
}

TEST_F(ActorTest, Cross_reference_prefab) {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    ASSERT_TRUE(prefab != nullptr);

    Actor *level1 = Engine::objectCreate<Actor>("Level1", prefab);
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(level1->addComponent("TestComponent"));
    ASSERT_TRUE(prefabTestComponent != nullptr);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    Actor *root = Engine::objectCreate<Actor>("Root");

    Actor *clone1 = dynamic_cast<Actor *>(prefab->clone(root));
    ASSERT_TRUE(clone1 != nullptr);

    TestComponent *cloneTestComponent1 = dynamic_cast<TestComponent *>(clone1->componentInChild("TestComponent"));
    ASSERT_TRUE(cloneTestComponent1 != nullptr);

    Actor *clone2 = dynamic_cast<Actor *>(prefab->clone(root));
    ASSERT_TRUE(clone2 != nullptr);

    TestComponent *cloneTestComponent2 = dynamic_cast<TestComponent *>(clone2->componentInChild("TestComponent"));
    ASSERT_TRUE(cloneTestComponent2 != nullptr);

    cloneTestComponent1->setReference(cloneTestComponent2);
    cloneTestComponent2->setReference(cloneTestComponent1);

    Variant data = Engine::toVariant(root);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    ASSERT_TRUE(result != nullptr);

    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(result->componentInChild("TestComponent"));
    ASSERT_TRUE(resultTestComponent != nullptr);
    TestComponent *referenceTestComponent = resultTestComponent->reference();

    ASSERT_TRUE(referenceTestComponent != nullptr);

    ASSERT_TRUE(referenceTestComponent != resultTestComponent);
    ASSERT_TRUE(resultTestComponent->reference() == referenceTestComponent);
    //QSKIP("Temporary disabled");
    //ASSERT_TRUE(referenceTestComponent->reference() == resultTestComponent);

    delete result;

    delete clone2;
    delete clone1;
    delete prefab;
}

TEST_F(ActorTest, Remove_component_from_prefab_instance) {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    ASSERT_TRUE(prefab != nullptr);

    Actor *level1 = Engine::objectCreate<Actor>("Level1", prefab);
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(level1->addComponent("TestComponent"));
    ASSERT_TRUE(prefabTestComponent != nullptr);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    ASSERT_TRUE(clone != nullptr);
    ASSERT_TRUE(clone->isInstance());

    TestComponent *cloneTestComponent = dynamic_cast<TestComponent *>(clone->componentInChild("TestComponent"));
    ASSERT_TRUE(cloneTestComponent != nullptr);

    delete cloneTestComponent;

    Variant data = Engine::toVariant(clone);

    Object *object = Engine::toObject(data);
    Actor *result = dynamic_cast<Actor *>(object);
    ASSERT_TRUE(result != nullptr);

    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(result->componentInChild("TestComponent"));
    ASSERT_TRUE(resultTestComponent == nullptr);

    delete result;

    delete clone;
    delete prefab;
}

TEST_F(ActorTest, Update_prefab_instance) {
    Engine system(nullptr, "");
    TestComponent::registerClassFactory(&system);

    Actor *prefab = Engine::objectCreate<Actor>("Prefab");
    ASSERT_TRUE(prefab != nullptr);

    Actor *level1 = Engine::composeActor("TestComponent", "Level1", prefab);

    uint32_t uuidLevel1 = level1->uuid();

    Actor *level2 = Engine::composeActor("", "Level2", prefab);

    Prefab *fab = Engine::objectCreate<Prefab>("");
    fab->setActor(prefab);

    system.resourceSystem()->setResource(fab, "TestPrefab");

    // Create an instance before do changes in prefab
    Actor *clone = dynamic_cast<Actor *>(prefab->clone());
    ASSERT_TRUE(clone != nullptr);
    ASSERT_TRUE(clone->isInstance());

    switchStatePrefab(*fab, ResourceState::Loading);

    // Step 1 - Add item to prefab
    TestComponent *prefabTestComponent = dynamic_cast<TestComponent *>(prefab->addComponent("TestComponent"));
    ASSERT_TRUE(prefabTestComponent != nullptr);

    // Step 2 - Delete item from prefab
    delete level1;

    // Step 3 - Change prefab property
    const Vector3 value(1.0f, 2.0f, 3.0f);
    level2->transform()->setPosition(value);

    // Sync instance with prefab
    switchStatePrefab(*fab, ResourceState::Ready);

    // Check instance state from Step 1
    TestComponent *resultTestComponent = dynamic_cast<TestComponent *>(clone->component("TestComponent"));
    ASSERT_TRUE(resultTestComponent != nullptr);
    ASSERT_TRUE(resultTestComponent->parent()->name() == "Prefab");

    // Check instance state from Step 2
    Actor *cloneLevel1 = dynamic_cast<Actor *>(Engine::findObject(uuidLevel1, clone));
    ASSERT_TRUE(cloneLevel1 == nullptr);

    // Check instance state from Step 3
    Actor *cloneLevel2 = dynamic_cast<Actor *>(Engine::findObject(level2->uuid(), clone));
    ASSERT_TRUE(cloneLevel2 != nullptr);
    ASSERT_TRUE(cloneLevel2->name() ==level2->name());
    ASSERT_TRUE(cloneLevel2->transform() != nullptr);
    ASSERT_TRUE(cloneLevel2->transform()->position() == value);

    delete prefab;
}
