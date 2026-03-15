#include "gtest/gtest.h"

#include "components/recttransform.h"
#include "components/frame.h"
#include "components/layout.h"

#include "uikit.h"

namespace UikitSuite {

    class RectTransformTest : public ::testing::Test {

    };

    TEST(RectTransformTest, AddRemoveChild) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* parent = Engine::composeActor<Frame>("Parent");
        Actor* child1 = Engine::composeActor<Frame>("Child1");

        Frame* parentFrame = parent->getComponent<Frame>();
        ASSERT_TRUE(parentFrame != nullptr);
        Frame* child1Frame = child1->getComponent<Frame>();
        ASSERT_TRUE(child1Frame != nullptr);

        RectTransform* parentRect = parentFrame->rectTransform();
        ASSERT_TRUE(parentRect != nullptr);
        RectTransform* child1Rect = child1Frame->rectTransform();
        ASSERT_TRUE(child1Rect != nullptr);

        child1->setParent(parent);
        ASSERT_EQ(parentRect, child1Rect->parentTransform());
        ASSERT_EQ(1, parentFrame->childWidgets().size());

        Actor* child2 = Engine::composeActor<Frame>("Child2", parent);
        Frame* child2Frame = child2->getComponent<Frame>();
        ASSERT_TRUE(child2Frame != nullptr);
        RectTransform* child2Rect = child2Frame->rectTransform();
        ASSERT_TRUE(child2Rect != nullptr);

        ASSERT_EQ(2, parentFrame->childWidgets().size());

        child1->setParent(nullptr);
        ASSERT_EQ(1, parentFrame->childWidgets().size());

        child2Rect->setParentTransform(nullptr);
        ASSERT_EQ(0, parentFrame->childWidgets().size());
    }

    TEST(RectTransformTest, CreateLayoutAndAddChilds) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* parent = Engine::composeActor<Frame>("Parent");
        Actor* child1 = Engine::composeActor<Frame>("Child1");
        Actor* child2 = Engine::composeActor<Frame>("Child2");

        Frame* parentFrame = parent->getComponent<Frame>();
        ASSERT_TRUE(parentFrame != nullptr);
        Frame* child1Frame = child1->getComponent<Frame>();
        ASSERT_TRUE(child1Frame != nullptr);
        Frame* child2Frame = child2->getComponent<Frame>();
        ASSERT_TRUE(child2Frame != nullptr);

        RectTransform* parentRect = parentFrame->rectTransform();
        ASSERT_TRUE(parentRect != nullptr);
        RectTransform* child1Rect = child1Frame->rectTransform();
        ASSERT_TRUE(child1Rect != nullptr);
        RectTransform* child2Rect = child2Frame->rectTransform();
        ASSERT_TRUE(child2Rect != nullptr);

        Layout* layout = new Layout;

        parentRect->setLayout(layout);
        child2Rect->setParentTransform(parentRect);
        child1Rect->setParentTransform(parentRect);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child2Rect));
        ASSERT_EQ(1, layout->indexOf(child1Rect));
        ASSERT_EQ(child2Rect, layout->transformAt(0));
        ASSERT_EQ(child1Rect, layout->transformAt(1));
    }

    TEST(RectTransformTest, AddChildsAndCreateLayout) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* parent = Engine::composeActor<Frame>("Parent");
        Actor* child1 = Engine::composeActor<Frame>("Child1", parent);
        Actor* child2 = Engine::composeActor<Frame>("Child2", parent);

        Frame* parentFrame = parent->getComponent<Frame>();
        ASSERT_TRUE(parentFrame != nullptr);
        Frame* child1Frame = child1->getComponent<Frame>();
        ASSERT_TRUE(child1Frame != nullptr);
        Frame* child2Frame = child2->getComponent<Frame>();
        ASSERT_TRUE(child2Frame != nullptr);

        RectTransform* parentRect = parentFrame->rectTransform();
        ASSERT_TRUE(parentRect != nullptr);
        RectTransform* child1Rect = child1Frame->rectTransform();
        ASSERT_TRUE(child1Rect != nullptr);
        RectTransform* child2Rect = child2Frame->rectTransform();
        ASSERT_TRUE(child2Rect != nullptr);

        Layout* layout = new Layout;

        parentRect->setLayout(layout);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child1Rect));
        ASSERT_EQ(1, layout->indexOf(child2Rect));
        ASSERT_EQ(child1Rect, layout->transformAt(0));
        ASSERT_EQ(child2Rect, layout->transformAt(1));
    }

    TEST(RectTransformTest, CreateLayoutAndInsertChild) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* parent = Engine::composeActor<Frame>("Parent");
        Actor* child1 = Engine::composeActor<Frame>("Child1");
        Actor* child2 = Engine::composeActor<Frame>("Child2", parent);

        Frame* parentFrame = parent->getComponent<Frame>();
        ASSERT_TRUE(parentFrame != nullptr);
        Frame* child1Frame = child1->getComponent<Frame>();
        ASSERT_TRUE(child1Frame != nullptr);
        Frame* child2Frame = child2->getComponent<Frame>();
        ASSERT_TRUE(child2Frame != nullptr);

        RectTransform* parentRect = parentFrame->rectTransform();
        ASSERT_TRUE(parentRect != nullptr);
        RectTransform* child1Rect = child1Frame->rectTransform();
        ASSERT_TRUE(child1Rect != nullptr);
        RectTransform* child2Rect = child2Frame->rectTransform();
        ASSERT_TRUE(child2Rect != nullptr);

        Layout* layout = new Layout;

        parentRect->setLayout(layout);
        layout->insertTransform(0, child1Rect);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child1Rect));
        ASSERT_EQ(1, layout->indexOf(child2Rect));
        ASSERT_EQ(child1Rect, layout->transformAt(0));
        ASSERT_EQ(child2Rect, layout->transformAt(1));

        // Move rect
        layout->insertTransform(0, child2Rect);
        ASSERT_EQ(0, layout->indexOf(child2Rect));
        ASSERT_EQ(1, layout->indexOf(child1Rect));
    }

    TEST(RectTransformTest, LayoutAutoFitParentSize) {
        RectTransform parentRect;
        parentRect.setVerticalPolicy(RectTransform::Preferred);
        parentRect.setBorder(Vector4(1));
        parentRect.setPadding(Vector4(1));
        parentRect.setSize(Vector2(100));

        RectTransform child1Rect;
        child1Rect.setParentTransform(&parentRect);
        child1Rect.setHorizontalPolicy(RectTransform::Preferred);
        child1Rect.setPivot(Vector2(0, 1));
        child1Rect.setSize(Vector2(10.0f, 100.0f));
        child1Rect.setMargin(Vector4(1));

        RectTransform child2Rect;
        child2Rect.setParentTransform(&parentRect);
        child2Rect.setHorizontalPolicy(RectTransform::Expanding);
        child2Rect.setPivot(Vector2(0, 1));
        child2Rect.setSize(Vector2(20.0f, 100.0f));
        child2Rect.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        layout->setSpacing(2);
        parentRect.setLayout(layout);

        ASSERT_EQ(100.0f, parentRect.size().x);
        ASSERT_EQ(210.0f, parentRect.size().y);
        ASSERT_EQ(10.0f, child1Rect.size().x);
        ASSERT_EQ(107.0f, child1Rect.position().y);
        ASSERT_EQ(94.0f, child2Rect.size().x);
        ASSERT_EQ(3.0f, child2Rect.position().y);

        // Hiding first child
        child1Rect.setEnabled(false);

        ASSERT_EQ(100.0f, parentRect.size().x);
        ASSERT_EQ(106.0f, parentRect.size().y);
        ASSERT_EQ(3.0f, child2Rect.position().y);
    }

    TEST(RectTransformTest, LayoutHorizontalPreferedExpand) {
        RectTransform parentRect;
        parentRect.setBorder(Vector4(1));
        parentRect.setPadding(Vector4(1));
        parentRect.setSize(Vector3(100.0f));

        RectTransform child1Rect;
        child1Rect.setParentTransform(&parentRect);
        child1Rect.setPivot(Vector2(0, 1));
        child1Rect.setHorizontalPolicy(RectTransform::Fixed);
        child1Rect.setVerticalPolicy(RectTransform::Expanding);
        child1Rect.setSize(Vector2(30.0f));
        child1Rect.setMargin(Vector4(1));

        RectTransform child2Rect;
        child2Rect.setParentTransform(&parentRect);
        child2Rect.setPivot(Vector2(0, 1));
        child2Rect.setHorizontalPolicy(RectTransform::Preferred);
        child2Rect.setVerticalPolicy(RectTransform::Expanding);
        child2Rect.setSize(Vector2(20.0f));
        child2Rect.setMargin(Vector4(1));

        RectTransform child3Rect;
        child3Rect.setParentTransform(&parentRect);
        child3Rect.setPivot(Vector2(0, 1));
        child3Rect.setHorizontalPolicy(RectTransform::Expanding);
        child3Rect.setVerticalPolicy(RectTransform::Expanding);
        child3Rect.setSize(Vector2(20.0f));
        child3Rect.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Horizontal);
        layout->setSpacing(1);
        parentRect.setLayout(layout);

        ASSERT_EQ(100.0f, parentRect.size().x);
        ASSERT_EQ( 3.0f, child1Rect.position().x);
        ASSERT_EQ(30.0f, child1Rect.size().x);
        ASSERT_EQ(94.0f, child1Rect.size().y);
        ASSERT_EQ(36.0f, child2Rect.position().x);
        ASSERT_EQ(20.0f, child2Rect.size().x);
        ASSERT_EQ(94.0f, child2Rect.size().y);
        ASSERT_EQ(59.0f, child3Rect.position().x);
        ASSERT_EQ(38.0f, child3Rect.size().x);
        ASSERT_EQ(94.0f, child3Rect.size().y);
    }

    TEST(RectTransformTest, LayoutVerticalPreferedExpand) {
        RectTransform parentRect;
        parentRect.setBorder(Vector4(1));
        parentRect.setPadding(Vector4(1));
        parentRect.setSize(Vector3(100.0f));

        RectTransform child1Rect;
        child1Rect.setParentTransform(&parentRect);
        child1Rect.setPivot(Vector2(0, 1));
        child1Rect.setHorizontalPolicy(RectTransform::Expanding);
        child1Rect.setVerticalPolicy(RectTransform::Fixed);
        child1Rect.setSize(Vector2(30.0f));
        child1Rect.setMargin(Vector4(1));

        RectTransform child2Rect;
        child2Rect.setParentTransform(&parentRect);
        child2Rect.setPivot(Vector2(0, 1));
        child2Rect.setHorizontalPolicy(RectTransform::Expanding);
        child2Rect.setVerticalPolicy(RectTransform::Preferred);
        child2Rect.setSize(Vector2(20.0f));
        child2Rect.setMargin(Vector4(1));

        RectTransform child3Rect;
        child3Rect.setParentTransform(&parentRect);
        child3Rect.setPivot(Vector2(0, 1));
        child3Rect.setHorizontalPolicy(RectTransform::Expanding);
        child3Rect.setVerticalPolicy(RectTransform::Expanding);
        child3Rect.setSize(Vector2(20.0f));
        child3Rect.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        layout->setSpacing(1);
        parentRect.setLayout(layout);

        ASSERT_EQ(100.0f, parentRect.size().y);
        ASSERT_EQ(67.0f, child1Rect.position().y);
        ASSERT_EQ(94.0f, child1Rect.size().x);
        ASSERT_EQ(30.0f, child1Rect.size().y);
        ASSERT_EQ(44.0f, child2Rect.position().y);
        ASSERT_EQ(94.0f, child2Rect.size().x);
        ASSERT_EQ(20.0f, child2Rect.size().y);
        ASSERT_EQ( 3.0f, child3Rect.position().y);
        ASSERT_EQ(94.0f, child3Rect.size().x);
        ASSERT_EQ(38.0f, child3Rect.size().y);
    }

}
