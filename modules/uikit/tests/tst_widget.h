/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "gtest/gtest.h"

#include <frame.h>
#include <layout.h>

#include <uikit.h>

namespace UikitSuite {

    class RectTransformTest : public ::testing::Test {

    };

    TEST_F(RectTransformTest, AddRemoveChild) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        Frame *child1 = Widget::createWidget<Frame>("Child1", parent);
        ASSERT_TRUE(child1 != nullptr);
        child1->setParent(parent);
        ASSERT_EQ(parent, child1->parentTransform());
        ASSERT_EQ(1, parent->childWidgets().size());

        Frame *child2 = Widget::createWidget<Frame>("Child2", parent);
        ASSERT_TRUE(child2 != nullptr);

        ASSERT_EQ(2, parent->childWidgets().size());

        child1->setParent(nullptr);
        ASSERT_EQ(1, parent->childWidgets().size());

        child2->setParent(nullptr);
        ASSERT_EQ(0, parent->childWidgets().size());
    }

    TEST_F(RectTransformTest, CreateLayoutAndAddChilds) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        Frame *child1 = Widget::createWidget<Frame>("Child1", parent);
        ASSERT_TRUE(child1 != nullptr);
        Frame *child2 = Widget::createWidget<Frame>("Child2", parent);
        ASSERT_TRUE(child2 != nullptr);

        Layout *layout = new Layout;
        parent->setLayout(layout);
        child2->setParent(parent);
        child1->setParent(parent);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child2));
        ASSERT_EQ(1, layout->indexOf(child1));
        ASSERT_EQ(child2, layout->widgetAt(0));
        ASSERT_EQ(child1, layout->widgetAt(1));
    }

    TEST_F(RectTransformTest, AddChildsAndCreateLayout) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        Frame *child1 = Widget::createWidget<Frame>("Child1", parent);
        ASSERT_TRUE(child1 != nullptr);
        Frame *child2 = Widget::createWidget<Frame>("Child2", parent);
        ASSERT_TRUE(child2 != nullptr);

        Layout *layout = new Layout;
        parent->setLayout(layout);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child1));
        ASSERT_EQ(1, layout->indexOf(child2));
        ASSERT_EQ(child1, layout->widgetAt(0));
        ASSERT_EQ(child2, layout->widgetAt(1));
    }

    TEST_F(RectTransformTest, CreateLayoutAndInsertChild) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        Frame *child1 = Widget::createWidget<Frame>("Child1");
        ASSERT_TRUE(child1 != nullptr);
        Frame *child2 = Widget::createWidget<Frame>("Child2", parent);
        ASSERT_TRUE(child2 != nullptr);

        Layout* layout = new Layout;
        parent->setLayout(layout);
        layout->insertWidget(0, child1);

        ASSERT_EQ(2, layout->count());
        ASSERT_EQ(0, layout->indexOf(child1));
        ASSERT_EQ(1, layout->indexOf(child2));
        ASSERT_EQ(child1, layout->widgetAt(0));
        ASSERT_EQ(child2, layout->widgetAt(1));

        // Move rect
        layout->insertTransform(0, child2);
        ASSERT_EQ(0, layout->indexOf(child2));
        ASSERT_EQ(1, layout->indexOf(child1));
    }

    TEST_F(RectTransformTest, LayoutAutoFitParentSize) {
        Widget parent;
        parent.setVerticalPolicy(Widget::Preferred);
        parent.setBorder(Vector4(1));
        parent.setPadding(Vector4(1));
        parent.setSize(Vector2(100));

        Widget child1;
        child1.setParentTransform(&parent);
        child1.setHorizontalPolicy(Widget::Preferred);
        child1.setPivot(Vector2(0, 1));
        child1.setSize(Vector2(10.0f, 100.0f));
        child1.setMargin(Vector4(1));

        Widget child2;
        child2.setParentTransform(&parent);
        child2.setHorizontalPolicy(Widget::Expanding);
        child2.setPivot(Vector2(0, 1));
        child2.setSize(Vector2(20.0f, 100.0f));
        child2.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        layout->setSpacing(2);
        parent.setLayout(layout);

        ASSERT_EQ(100.0f, parent.size().x);
        ASSERT_EQ(210.0f, parent.size().y);
        ASSERT_EQ(10.0f, child1.size().x);
        ASSERT_EQ(107.0f, child1.position().y);
        ASSERT_EQ(94.0f, child2.size().x);
        ASSERT_EQ(3.0f, child2.position().y);

        // Hide first child
        child1.setEnabled(false);

        ASSERT_EQ(100.0f, parent.size().x);
        ASSERT_EQ(106.0f, parent.size().y);
        ASSERT_EQ(3.0f, child2.position().y);
    }

    TEST_F(RectTransformTest, LayoutHorizontalPreferedExpand) {
        Widget parent;
        parent.setBorder(Vector4(1));
        parent.setPadding(Vector4(1));
        parent.setSize(Vector2(100.0f));

        Widget child1;
        child1.setParentTransform(&parent);
        child1.setPivot(Vector2(0, 1));
        child1.setHorizontalPolicy(Widget::Fixed);
        child1.setVerticalPolicy(Widget::Expanding);
        child1.setSize(Vector2(30.0f));
        child1.setMargin(Vector4(1));

        Widget child2;
        child2.setParentTransform(&parent);
        child2.setPivot(Vector2(0, 1));
        child2.setHorizontalPolicy(Widget::Preferred);
        child2.setVerticalPolicy(Widget::Expanding);
        child2.setSize(Vector2(20.0f));
        child2.setMargin(Vector4(1));

        Widget child3;
        child3.setParentTransform(&parent);
        child3.setPivot(Vector2(0, 1));
        child3.setHorizontalPolicy(Widget::Expanding);
        child3.setVerticalPolicy(Widget::Expanding);
        child3.setSize(Vector2(20.0f));
        child3.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Horizontal);
        layout->setSpacing(1);
        parent.setLayout(layout);

        ASSERT_EQ(100.0f, parent.size().x);
        ASSERT_EQ( 3.0f, child1.position().x);
        ASSERT_EQ(30.0f, child1.size().x);
        ASSERT_EQ(94.0f, child1.size().y);
        ASSERT_EQ(36.0f, child2.position().x);
        ASSERT_EQ(20.0f, child2.size().x);
        ASSERT_EQ(94.0f, child2.size().y);
        ASSERT_EQ(59.0f, child3.position().x);
        ASSERT_EQ(38.0f, child3.size().x);
        ASSERT_EQ(94.0f, child3.size().y);
    }

    TEST_F(RectTransformTest, LayoutVerticalPreferedExpand) {
        Widget parent;
        parent.setBorder(Vector4(1));
        parent.setPadding(Vector4(1));
        parent.setSize(Vector2(100.0f));

        Widget child1;
        child1.setParentTransform(&parent);
        child1.setPivot(Vector2(0, 1));
        child1.setHorizontalPolicy(Widget::Expanding);
        child1.setVerticalPolicy(Widget::Fixed);
        child1.setSize(Vector2(30.0f));
        child1.setMargin(Vector4(1));

        Widget child2;
        child2.setParentTransform(&parent);
        child2.setPivot(Vector2(0, 1));
        child2.setHorizontalPolicy(Widget::Expanding);
        child2.setVerticalPolicy(Widget::Preferred);
        child2.setSize(Vector2(20.0f));
        child2.setMargin(Vector4(1));

        Widget child3;
        child3.setParentTransform(&parent);
        child3.setPivot(Vector2(0, 1));
        child3.setHorizontalPolicy(Widget::Expanding);
        child3.setVerticalPolicy(Widget::Expanding);
        child3.setSize(Vector2(20.0f));
        child3.setMargin(Vector4(1));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        layout->setSpacing(1);
        parent.setLayout(layout);

        ASSERT_EQ(100.0f, parent.size().y);
        ASSERT_EQ(67.0f, child1.position().y);
        ASSERT_EQ(94.0f, child1.size().x);
        ASSERT_EQ(30.0f, child1.size().y);
        ASSERT_EQ(44.0f, child2.position().y);
        ASSERT_EQ(94.0f, child2.size().x);
        ASSERT_EQ(20.0f, child2.size().y);
        ASSERT_EQ( 3.0f, child3.position().y);
        ASSERT_EQ(94.0f, child3.size().x);
        ASSERT_EQ(38.0f, child3.size().y);
    }

    TEST_F(RectTransformTest, LayoutVerticalZeroExpand) {
        Widget parent;
        parent.setSize(Vector2(100.0f));

        Layout* layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        parent.setLayout(layout);

        Widget child1;
        child1.setHorizontalPolicy(Widget::Expanding);
        child1.setVerticalPolicy(Widget::Expanding);

        Widget child2;
        child2.setHorizontalPolicy(Widget::Expanding);
        child2.setVerticalPolicy(Widget::Expanding);

        layout->addTransform(&child1);
        layout->addTransform(&child2);

        ASSERT_EQ(50.0f, child1.size().y);
        ASSERT_EQ(50.0f, child2.size().y);
    }
}
