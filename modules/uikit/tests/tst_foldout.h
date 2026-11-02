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

#include <checkbox.h>
#include <foldout.h>
#include <frame.h>
#include <label.h>
#include <layout.h>

#include <uikit.h>

namespace UikitSuite {

    class FoldoutTest : public ::testing::Test {

    };

    TEST_F(FoldoutTest, ExpansionRepositionsFollowingWidgets) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Frame *parent = Widget::createWidget<Frame>("Parent");
        ASSERT_TRUE(parent != nullptr);
        parent->setSize(Vector2(100.0f, 200.0f));

        Foldout *foldout = Widget::createWidget<Foldout>("Foldout", parent);
        ASSERT_TRUE(foldout != nullptr);
        foldout->setExpanded(false);

        Frame *content = Widget::createWidget<Frame>("Content");
        ASSERT_TRUE(content != nullptr);
        content->setSize(Vector2(100.0f, 40.0f));
        foldout->insertWidget(-1, content);

        Frame *following = Widget::createWidget<Frame>("Following", parent);
        ASSERT_TRUE(following != nullptr);
        following->setSize(Vector2(100.0f, 20.0f));

        Layout *layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        parent->setLayout(layout);

        const float collapsedY = following->position().y;
        const float collapsedHeight = foldout->size().y;

        foldout->setExpanded(true);

        EXPECT_FLOAT_EQ(collapsedHeight + content->size().y, foldout->size().y);
        EXPECT_FLOAT_EQ(collapsedY - content->size().y, following->position().y);

        foldout->setExpanded(false);

        EXPECT_FLOAT_EQ(collapsedHeight, foldout->size().y);
        EXPECT_FLOAT_EQ(collapsedY, following->position().y);
    }

    TEST_F(FoldoutTest, HeaderUsesSingleHeaderWidget) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Foldout *foldout = Widget::createWidget<Foldout>("Foldout");
        ASSERT_TRUE(foldout != nullptr);
        foldout->setText("Foldout title");

        CheckBox *header = foldout->header();
        Label *label = foldout->label();
        ASSERT_TRUE(header != nullptr);
        ASSERT_TRUE(label != nullptr);
        ASSERT_TRUE(header->layout() != nullptr);

        EXPECT_EQ(foldout, header->parentWidget());
        EXPECT_EQ(header, label->parentWidget());
        EXPECT_EQ(Widget::Horizontal, header->layout()->orientation());
        EXPECT_GE(header->layout()->indexOf(label), 0);

        Frame *control = Widget::createWidget<Frame>("Control");
        ASSERT_TRUE(control != nullptr);
        foldout->insertHeaderWidget(-1, control);

        EXPECT_EQ(header, control->parentWidget());
        EXPECT_GE(header->layout()->indexOf(control), 0);
        EXPECT_TRUE(header->isHovered(Vector2(header->worldTransform()[12] + 8.0f,
                                              header->worldTransform()[13] + 10.0f)));
    }

}
