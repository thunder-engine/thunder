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

#include <canvas.h>
#include <uikit.h>

namespace UikitSuite {

    class CanvasTest : public ::testing::Test {
    };

    TEST_F(CanvasTest, ContentScaleMapsInputCoordinates) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Canvas canvas;
        canvas.setSize(Vector2(200.0f, 100.0f));
        canvas.setPosition(Vector3(10.0f, 20.0f, 0.0f));
        canvas.setContentScale(2.0f);

        Vector2 contentPosition(canvas.mapToContent(Vector2(50.0f, 60.0f)));
        EXPECT_FLOAT_EQ(30.0f, contentPosition.x);
        EXPECT_FLOAT_EQ(40.0f, contentPosition.y);

        Vector2 screenPosition(canvas.mapFromContent(contentPosition));
        EXPECT_FLOAT_EQ(50.0f, screenPosition.x);
        EXPECT_FLOAT_EQ(60.0f, screenPosition.y);
    }

    TEST_F(CanvasTest, SnapsTextOriginToScaledPixelGrid) {
        Engine engine;
        engine.addModule(new UiKit(&engine));

        Canvas canvas;
        canvas.setPosition(Vector3(10.0f, 20.0f, 0.0f));
        canvas.setContentScale(2.0f);

        Matrix4 transform;
        transform[12] = 20.2f;
        transform[13] = 40.3f;

        Matrix4 snapped(canvas.snapTextTransform(transform));
        EXPECT_FLOAT_EQ(20.0f, snapped[12]);
        EXPECT_FLOAT_EQ(40.5f, snapped[13]);
    }

}
