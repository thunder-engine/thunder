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
#include <tabbar.h>
#include <tabwidget.h>
#include <uikit.h>

namespace UikitSuite {

    class TabWidgetTest : public ::testing::Test {
    };

    TEST_F(TabWidgetTest, MovingTabsKeepsCurrentContentSelected) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        TabWidget *tabs = Widget::createWidget<TabWidget>("Tabs");
        ASSERT_TRUE(tabs != nullptr);
        Frame *first = Widget::createWidget<Frame>("First");
        Frame *second = Widget::createWidget<Frame>("Second");
        Frame *third = Widget::createWidget<Frame>("Third");
        ASSERT_TRUE(first != nullptr);
        ASSERT_TRUE(second != nullptr);
        ASSERT_TRUE(third != nullptr);
        tabs->addTab("First", first);
        tabs->addTab("Second", second);
        tabs->addTab("Third", third);
        tabs->setCurrentIndex(1);
        tabs->setMovable(true);

        TabBar *bar = nullptr;
        for(Widget *child : tabs->childWidgets()) {
            bar = dynamic_cast<TabBar *>(child);
            if(bar) {
                break;
            }
        }
        ASSERT_TRUE(bar != nullptr);
        EXPECT_TRUE(tabs->isMovable());

        bar->moveTab(0, 2);

        EXPECT_EQ(third, tabs->tabContent(1));
        EXPECT_EQ(first, tabs->tabContent(2));
        EXPECT_EQ(second, tabs->tabContent(tabs->currentIndex()));
        EXPECT_EQ(0, tabs->currentIndex());
        EXPECT_EQ(0, bar->currentIndex());
    }

}
