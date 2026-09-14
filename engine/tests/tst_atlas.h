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

#include "utils/atlas.h"

namespace EngineSuite {

    TEST(AtlasTest, Default_state) {
        AtlasNode atlas;

        ASSERT_EQ(nullptr, atlas.left);
        ASSERT_EQ(nullptr, atlas.right);
        ASSERT_EQ(nullptr, atlas.parent);
        ASSERT_EQ(0u, atlas.x);
        ASSERT_EQ(0u, atlas.y);
        ASSERT_EQ(1u, atlas.w);
        ASSERT_EQ(1u, atlas.h);
        ASSERT_FALSE(atlas.occupied);
    }

    TEST(AtlasTest, Exact_fit_returns_leaf_without_splitting) {
        AtlasNode atlas;
        atlas.w = 16;
        atlas.h = 8;

        AtlasNode *result = atlas.insert(16, 8);

        ASSERT_EQ(&atlas, result);
        ASSERT_EQ(nullptr, atlas.left);
        ASSERT_EQ(nullptr, atlas.right);
    }

    TEST(AtlasTest, Rejects_region_that_does_not_fit) {
        AtlasNode atlas;
        atlas.w = 16;
        atlas.h = 8;

        ASSERT_EQ(nullptr, atlas.insert(17, 8));
        ASSERT_EQ(nullptr, atlas.insert(16, 9));
        ASSERT_EQ(nullptr, atlas.left);
        ASSERT_EQ(nullptr, atlas.right);
    }

    TEST(AtlasTest, Splits_horizontally) {
        AtlasNode atlas;
        atlas.w = 8;
        atlas.h = 4;

        AtlasNode *result = atlas.insert(3, 4);

        ASSERT_EQ(atlas.left, result);
        ASSERT_EQ(0u, result->x);
        ASSERT_EQ(0u, result->y);
        ASSERT_EQ(3u, result->w);
        ASSERT_EQ(4u, result->h);
        ASSERT_EQ(3u, atlas.right->x);
        ASSERT_EQ(0u, atlas.right->y);
        ASSERT_EQ(5u, atlas.right->w);
        ASSERT_EQ(4u, atlas.right->h);
        ASSERT_EQ(&atlas, result->parent);
        ASSERT_EQ(&atlas, atlas.right->parent);
    }

    TEST(AtlasTest, Splits_vertically) {
        AtlasNode atlas;
        atlas.w = 4;
        atlas.h = 8;

        AtlasNode *result = atlas.insert(4, 3);

        ASSERT_EQ(atlas.left, result);
        ASSERT_EQ(0u, result->x);
        ASSERT_EQ(0u, result->y);
        ASSERT_EQ(4u, result->w);
        ASSERT_EQ(3u, result->h);
        ASSERT_EQ(0u, atlas.right->x);
        ASSERT_EQ(3u, atlas.right->y);
        ASSERT_EQ(4u, atlas.right->w);
        ASSERT_EQ(5u, atlas.right->h);
    }

    TEST(AtlasTest, Skips_occupied_region_and_uses_right_sibling) {
        AtlasNode atlas;
        atlas.w = 8;
        atlas.h = 4;

        AtlasNode *first = atlas.insert(3, 4);
        ASSERT_TRUE(first != nullptr);
        first->occupied = true;

        AtlasNode *second = atlas.insert(5, 4);

        ASSERT_EQ(atlas.right, second);
        ASSERT_EQ(3u, second->x);
        ASSERT_EQ(5u, second->w);
        ASSERT_EQ(4u, second->h);
    }

    TEST(AtlasTest, Clean_removes_empty_split_children) {
        AtlasNode atlas;
        atlas.w = 8;
        atlas.h = 4;

        AtlasNode *first = atlas.insert(3, 2);
        first->occupied = true;
        AtlasNode *second = atlas.insert(3, 2);
        ASSERT_TRUE(first != nullptr);
        ASSERT_TRUE(second != nullptr);
        ASSERT_TRUE(first != second);
        ASSERT_TRUE(atlas.left->left != nullptr);
        ASSERT_TRUE(atlas.left->right != nullptr);

        first->occupied = true;
        second->occupied = true;

        atlas.left->clean();
        ASSERT_TRUE(atlas.left->left != nullptr);
        ASSERT_TRUE(atlas.left->right != nullptr);

        first->occupied = false;
        second->occupied = false;
        atlas.left->clean();
        ASSERT_EQ(nullptr, atlas.left->left);
        ASSERT_EQ(nullptr, atlas.left->right);
    }

}
