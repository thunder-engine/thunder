#include "tst_common.h"

#include "components/actor.h"
#include "components/transform.h"

namespace EngineSuite {

    class TransformTest : public ::testing::Test {
    };

    TEST_F(TransformTest, Hierarchy) {
        ObjectSystem system;
        Actor::registerClassFactory(&system);
        Transform::registerClassFactory(&system);

        Actor parent;
        parent.addComponent("Transform");

        Actor child;
        child.addComponent("Transform");

        Transform *parentTransform = parent.transform();
        Transform *childTransform = child.transform();

        ASSERT_TRUE(parentTransform != nullptr);
        ASSERT_TRUE(childTransform != nullptr);

        child.setParent(&parent);

        ASSERT_TRUE(childTransform->parentTransform() == parentTransform);
    }

    TEST_F(TransformTest, Reparent_to_root_preserves_world_transform) {
        ObjectSystem system;
        Actor::registerClassFactory(&system);
        Transform::registerClassFactory(&system);

        Actor parent;
        parent.addComponent("Transform");
        parent.transform()->setPosition(Vector3(0.0f, 5.0f, 0.0f));
        parent.transform()->setRotation(Vector3(-50.0f, 130.0f, 0.0f));

        Actor child;
        child.addComponent("Transform");
        child.transform()->setPosition(Vector3(2.0f, 1.0f, -3.0f));
        child.transform()->setRotation(Vector3(15.0f, 25.0f, 35.0f));
        child.transform()->setScale(Vector3(1.5f, 2.0f, 0.75f));

        child.setParent(&parent);

        Transform *transform = child.transform();
        Vector3 worldPosition = transform->worldPosition();
        Vector3 worldRotation = transform->worldRotation();
        Vector3 worldScale = transform->worldScale();

        child.setParent(nullptr);

        ASSERT_TRUE(transform->parentTransform() == nullptr);
        ASSERT_TRUE(transform->worldPosition() == worldPosition);
        ASSERT_TRUE(transform->worldRotation() == worldRotation);
        ASSERT_TRUE(transform->worldScale() == worldScale);
    }

}