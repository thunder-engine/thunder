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

#include "components/splitter.h"
#include "uikit.h"

namespace UikitSuite {

    class SplitterTest : public ::testing::Test {

    };

    TEST_F(SplitterTest, AddWidgets) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Splitter *splitter = Widget::createWidget<Splitter>("Test");
        if (splitter) {
            Frame *frame1 = Widget::createWidget<Frame>("Frame1");

            Frame *frame2 = Widget::createWidget<Frame>("Frame2");

            splitter->addWidget(frame1);
            splitter->addWidget(frame2);

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame1, splitter->widget(0));
            ASSERT_EQ(frame2, splitter->widget(1));
            ASSERT_EQ(nullptr, splitter->widget(2));
            ASSERT_EQ(nullptr, splitter->widget(-1));

            ASSERT_EQ(1, splitter->indexOf(frame2));
            ASSERT_EQ(-1, splitter->indexOf(nullptr));
        }
    }

    TEST_F(SplitterTest, InsertWidget) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Splitter *splitter = Widget::createWidget<Splitter>("Test");
        if (splitter) {
            Frame *frame1 = Widget::createWidget<Frame>("Frame1");

            Frame *frame2 = Widget::createWidget<Frame>("Frame2");

            Frame *frame3 = Widget::createWidget<Frame>("Frame3");

            splitter->addWidget(frame1);
            splitter->addWidget(frame3);
            splitter->insertWidget(1, frame2);

            ASSERT_EQ(3, splitter->count());
            ASSERT_EQ(frame2, splitter->widget(1));
            ASSERT_EQ(frame3, splitter->widget(2));
        }
    }

    TEST_F(SplitterTest, ReplaceWidget) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Splitter *splitter = Widget::createWidget<Splitter>("Test");
        if (splitter) {
            Frame *frame1 = Widget::createWidget<Frame>("Frame1");

            Frame *frame2 = Widget::createWidget<Frame>("Frame2");

            Frame *frame3 = Widget::createWidget<Frame>("Frame3");

            splitter->addWidget(frame1);
            splitter->addWidget(frame3);
            Widget* widget = splitter->replaceWidget(1, frame2);

            ASSERT_EQ(frame3, widget);
            frame3->setParent(nullptr);

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame2, splitter->widget(1));
        }
    }

    TEST_F(SplitterTest, AddChild) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Splitter *splitter = Widget::createWidget<Splitter>("Test");
        if (splitter) {
            Frame *frame1 = Widget::createWidget<Frame>("Frame1", splitter);

            Frame *frame2 = Widget::createWidget<Frame>("Frame2", splitter);

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame1, splitter->widget(0));
            ASSERT_EQ(frame2, splitter->widget(1));
        }
    }
}
