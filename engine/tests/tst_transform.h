#include "tst_common.h"

#include "components/actor.h"
#include "components/transform.h"

namespace EngineSuite {

    class TransformTest : public ::testing::Test {
    };

    TEST_F(TransformTest, Default_properties_and_local_transform) {
        Transform transform;

        ASSERT_EQ(Vector3(), transform.position());
        ASSERT_EQ(Vector3(), transform.rotation());
        ASSERT_EQ(Vector3(1.0f), transform.scale());
        ASSERT_TRUE(transform.quaternion().equal(Quaternion()));
        ASSERT_EQ(nullptr, transform.parentTransform());
        ASSERT_TRUE(transform.children().empty());

        transform.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        transform.setScale(Vector3(2.0f, 3.0f, 4.0f));

        ASSERT_EQ(Vector3(1.0f, 2.0f, 3.0f), transform.localTransform().position());
        ASSERT_EQ(Vector3(1.0f, 2.0f, 3.0f), transform.worldPosition());
        ASSERT_EQ(Vector3(2.0f, 3.0f, 4.0f), transform.worldScale());
    }

    TEST_F(TransformTest, World_properties_follow_parent_and_dirty_updates) {
        Transform parent;
        parent.setPosition(Vector3(10.0f, 20.0f, 30.0f));
        parent.setRotation(Vector3(5.0f, 10.0f, 15.0f));
        parent.setScale(Vector3(2.0f, 3.0f, 4.0f));

        Transform child;
        child.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        child.setRotation(Vector3(2.0f, 4.0f, 6.0f));
        child.setScale(Vector3(0.5f, 2.0f, 1.5f));
        child.setParentTransform(&parent);

        ASSERT_EQ(Vector3(12.0f, 26.0f, 42.0f), child.worldPosition());
        ASSERT_EQ(Vector3(7.0f, 14.0f, 21.0f), child.worldRotation());
        ASSERT_EQ(Vector3(1.0f, 6.0f, 6.0f), child.worldScale());

        parent.setPosition(Vector3(20.0f, 30.0f, 40.0f));

        ASSERT_EQ(Vector3(22.0f, 36.0f, 52.0f), child.worldPosition());
    }

    TEST_F(TransformTest, Quaternion_properties_follow_parent) {
        Transform quaternionTransform;
        Quaternion quaternion(Vector3(45.0f, 0.0f, 0.0f));
        quaternionTransform.setQuaternion(quaternion);

        ASSERT_TRUE(quaternionTransform.quaternion().equal(quaternion));
        ASSERT_TRUE(quaternionTransform.worldQuaternion().equal(quaternion));

        Transform parent;
        parent.setRotation(Vector3(0.0f, 90.0f, 0.0f));

        Transform child;
        child.setRotation(Vector3(45.0f, 0.0f, 0.0f));
        child.setParentTransform(&parent);

        ASSERT_TRUE(child.worldQuaternion().equal(parent.worldQuaternion() * child.quaternion()));
    }

    TEST_F(TransformTest, Reparent_preserves_world_properties) {
        Transform firstParent;
        firstParent.setPosition(Vector3(10.0f, 20.0f, 30.0f));
        firstParent.setScale(Vector3(2.0f));

        Transform secondParent;
        secondParent.setPosition(Vector3(-10.0f, -20.0f, -30.0f));
        secondParent.setScale(Vector3(4.0f));

        Transform child;
        child.setPosition(Vector3(1.0f, 2.0f, 3.0f));
        child.setRotation(Vector3(5.0f, 10.0f, 15.0f));
        child.setScale(Vector3(0.5f, 1.0f, 1.5f));
        child.setParentTransform(&firstParent);

        Vector3 worldPosition = child.worldPosition();
        Vector3 worldRotation = child.worldRotation();
        Vector3 worldScale = child.worldScale();
        Quaternion worldQuaternion = child.worldQuaternion();

        child.setParentTransform(&secondParent);

        ASSERT_EQ(worldPosition, child.worldPosition());
        ASSERT_EQ(worldRotation, child.worldRotation());
        ASSERT_EQ(worldScale, child.worldScale());
        ASSERT_TRUE(worldQuaternion.equal(child.worldQuaternion()));
    }

    TEST_F(TransformTest, Parent_tracks_children_and_rejects_self) {
        Transform parent;
        Transform child;

        child.setParentTransform(&child);
        ASSERT_EQ(nullptr, child.parentTransform());

        child.setParentTransform(&parent);
        ASSERT_EQ(&parent, child.parentTransform());
        ASSERT_EQ(1u, parent.children().size());
        ASSERT_EQ(&child, parent.children().front());

        child.setParentTransform(nullptr);
        ASSERT_EQ(nullptr, child.parentTransform());
        ASSERT_TRUE(parent.children().empty());
    }

    TEST_F(TransformTest, Hash_changes_when_world_transform_changes) {
        Transform transform;

        uint32_t initialHash = transform.hash();
        transform.setPosition(Vector3(1.0f, 0.0f, 0.0f));

        ASSERT_NE(initialHash, transform.hash());
    }

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