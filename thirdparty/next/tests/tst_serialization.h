#include "tst_common.h"

#include "objectsystem.h"
#include "bson.h"
#include "json.h"

namespace Next {

    class SerializationTest : public ::testing::Test {
    public:
        VariantMap var1;

        void SetUp() {
            var1["bool"] = true;
            var1["str"] = "str";
            var1["int"] = 2;
            var1["float"] = 3.0f;

            VariantMap m;
            m["bool"] = true;
            m["str"] = "true";
            m["int"] = 1;

            var1["map"] = m;

            VariantList a;
            a.push_back("string");
            a.push_back(1.0f);
            a.push_back(123);

            var1["array"] = a;

            var1["vec2"] = Vector2(1);
            var1["vec3"] = Vector3(2);
            var1["vec4"] = Vector4(3);
            var1["quat"] = Quaternion();
            var1["mat3"] = Matrix3();
            var1["mat4"] = Matrix4();
        }

        void TearDown() {
            var1.clear();
        }

    };

    TEST_F(SerializationTest, Json_Serialize_Desirialize) {
        ASSERT_TRUE(Variant(var1) == Json::load(Json::save(var1, 0)));
    }

    TEST_F(SerializationTest, Bson_Serialize_Desirialize) {
        ByteArray bin = { '\x00','\x01','\x02','\x03','\x04' };
        var1["bin"] = bin;

        ASSERT_TRUE(Variant(var1) == Bson::load(Bson::save(var1), MetaType::VARIANTMAP));
    }
}
