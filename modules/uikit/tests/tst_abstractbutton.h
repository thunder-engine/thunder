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

#include <abstractbutton.h>

namespace UikitSuite {

    class AbstractButtonTest : public ::testing::Test {
    };

    TEST_F(AbstractButtonTest, ExclusiveButtonsUncheckExclusiveSiblingsOnly) {
        Widget parent;
        AbstractButton first;
        AbstractButton second;
        AbstractButton independent;
        first.setParent(&parent);
        second.setParent(&parent);
        independent.setParent(&parent);

        first.setCheckable(true);
        first.setExclusive(true);
        second.setCheckable(true);
        second.setExclusive(true);
        independent.setCheckable(true);
        independent.setChecked(true);

        first.setChecked(true);
        EXPECT_TRUE(first.isChecked());
        EXPECT_FALSE(second.isChecked());
        EXPECT_TRUE(independent.isChecked());

        second.setChecked(true);
        EXPECT_FALSE(first.isChecked());
        EXPECT_TRUE(second.isChecked());
        EXPECT_TRUE(independent.isChecked());
    }

}
