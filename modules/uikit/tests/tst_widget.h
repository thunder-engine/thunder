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
#include <uikit.h>

namespace UikitSuite {

    class WidgetTest : public ::testing::Test {
    };

    TEST_F(WidgetTest, AddRemoveChild) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        Frame *child1 = Widget::createWidget<Frame>("Child1", parent);
        ASSERT_TRUE(child1 != nullptr);
        child1->setParent(parent);
        ASSERT_EQ(parent, child1->parentWidget());
        ASSERT_EQ(1, parent->childWidgets().size());

        Frame *child2 = Widget::createWidget<Frame>("Child2", parent);
        ASSERT_TRUE(child2 != nullptr);

        ASSERT_EQ(2, parent->childWidgets().size());

        child1->setParent(nullptr);
        ASSERT_EQ(1, parent->childWidgets().size());

        child2->setParent(nullptr);
        ASSERT_EQ(0, parent->childWidgets().size());
    }

    TEST_F(WidgetTest, AnchorsRecalculateSizeAndPositionImmediately) {
        Widget parent;
        parent.setSize(Vector2(200.0f, 100.0f));

        Widget child;
        child.setParent(&parent);
        child.setMargin(Vector4(2.0f, 3.0f, 4.0f, 5.0f));
        child.setAnchors(Vector2(0.25f, 0.1f), Vector2(0.75f, 0.6f));

        EXPECT_FLOAT_EQ(92.0f, child.size().x);
        EXPECT_FLOAT_EQ(44.0f, child.size().y);

        parent.setSize(Vector2(300.0f, 120.0f));

        EXPECT_FLOAT_EQ(142.0f, child.size().x);
        EXPECT_FLOAT_EQ(54.0f, child.size().y);

        child.setAnchors(Vector2(0.5f, 0.25f), Vector2(0.75f, 0.75f));

        EXPECT_FLOAT_EQ(67.0f, child.size().x);
        EXPECT_FLOAT_EQ(54.0f, child.size().y);
        EXPECT_FLOAT_EQ(155.0f, child.worldTransform()[12]);
        EXPECT_FLOAT_EQ(34.0f, child.worldTransform()[13]);
    }

    TEST_F(WidgetTest, MinimumSizeIsRespectedByAnchorsAndLayout) {
        Widget parent;
        parent.setSize(Vector2(80.0f, 70.0f));

        Widget child;
        child.setMinimumSize(Vector2(100.0f));
        child.setAnchors(Vector2(0.0f), Vector2(1.0f));
        child.setParent(&parent);

        EXPECT_FLOAT_EQ(100.0f, child.size().x);
        EXPECT_FLOAT_EQ(100.0f, child.size().y);

        child.setSize(Vector2(50.0f));
        EXPECT_FLOAT_EQ(100.0f, child.size().x);
        EXPECT_FLOAT_EQ(100.0f, child.size().y);
    }

    TEST_F(WidgetTest, ScaleAppliesToWorldTransform) {
        Widget parent;
        parent.setScale(2.0f);

        Widget child;
        child.setParent(&parent);
        child.setScale(0.5f);

        EXPECT_FLOAT_EQ(1.0f, child.worldScale());
        EXPECT_FLOAT_EQ(1.0f, child.worldTransform()[0]);
    }

}
