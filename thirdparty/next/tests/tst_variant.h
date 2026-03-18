/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next. If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2026 Evgeniy Prikazchikov
*/

#include "tst_common.h"

#include "variant.h"

#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <iomanip>
#include <codecvt>

namespace NextSuite {
    class VariantTest : public ::testing::Test {

    };

    TEST_F(VariantTest, Set_Get_Basic_Check) {
        {
            Variant value = true;
            bool result = value.toBool();
            ASSERT_TRUE(result == true);
        }
        {
            Variant value = 5;
            int result = value.toInt();
            ASSERT_TRUE(result == 5);
        }
        {
            Variant value = 5.0f;
            float result = value.toFloat();
            ASSERT_TRUE(result == 5.0f);
        }
        {
            Variant value = "5";
            std::string result = value.toString().toStdString();
            ASSERT_TRUE(result == std::string("5"));
        }
        {
            Variant value = TString("5");
            std::string result = value.toString().toStdString();
            ASSERT_TRUE(result.c_str() == std::string("5"));
        }
    }

    TEST_F(VariantTest, Set_Get_Advanced_Check) {
        {
            Vector2 vector(1.0f, 2.0f);
            Variant value = vector;
            ASSERT_TRUE(value.toVector2() == vector);
        }
        {
            Vector3 vector(1.0f, 2.0f, 3.0f);
            Variant value = vector;
            ASSERT_TRUE(value.toVector3() == vector);
        }
        {
            Vector4 vector(1.0f, 2.0f, 3.0f, 4.0f);
            Variant value = vector;
            ASSERT_TRUE(value.toVector4() == vector);
        }
        {
            Quaternion quaternion(Vector3(1.0f, 2.0f, 3.0f), 4.0f);
            Variant value = quaternion;
            ASSERT_TRUE(value.toQuaternion() == quaternion);
        }
        {
            Matrix3 matrix;
            Variant value = matrix;
            ASSERT_TRUE(value.toMatrix3() == matrix);
        }
        {
            Matrix4 matrix;
            Variant value = matrix;
            ASSERT_TRUE(value.toMatrix4() == matrix);
        }
    }

    TEST_F(VariantTest, Convert_String_to_Bool_Int_Float) {
        {
            Variant value = "true";
            ASSERT_TRUE(value.toBool() == true);
        }
        {
            Variant value = "9.6";
            ASSERT_TRUE(value.toInt() == 9);
        }
    }

    TEST_F(VariantTest, Compare_Variants_Basic) {
        {
            Variant value1(true);
            Variant value2(false);

            ASSERT_TRUE(value1 == value1);
            ASSERT_FALSE(value1 == value2);
        }
        {
            Variant value1(0);
            Variant value2(1);

            ASSERT_TRUE(value1 == value1);
            ASSERT_FALSE(value1 == value2);
        }
        {
            Variant value1(0.0f);
            Variant value2(1.0f);

            ASSERT_TRUE(value1 == value1);
            ASSERT_FALSE(value1 == value2);
        }
        {
            Variant value1("test1");
            Variant value2("test2");

            ASSERT_TRUE(value1 == value1);
            ASSERT_FALSE(value1 == value2);
        }
    }

    TEST_F(VariantTest, Compare_Variants_Advanced) {
        {
            VariantList value1;
            value1.push_back("Test");
            value1.push_back(true);
            VariantList value2;
            value2.push_back("Test");
            value2.push_back(false);

            ASSERT_TRUE(Variant(value1) == Variant(value1));
            ASSERT_FALSE(Variant(value1) == Variant(value2));
        }
        {
            VariantMap value1;
            value1["NAME"] = "Test";
            value1["VALUE"] = true;
            VariantMap value2;
            value2["NAME"] = "Test";
            value2["VALUE"] = false;

            ASSERT_TRUE(Variant(value1) == Variant(value1));
            ASSERT_FALSE(Variant(value1) == Variant(value2));
        }
    }
}
