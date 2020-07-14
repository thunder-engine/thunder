#include "tst_common.h"

#include "variant.h"

#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <iomanip>
#include <codecvt>

class VariantTest : public QObject {
    Q_OBJECT

private slots:

void Set_Get_Basic_Check() {
    {
        Variant value   = true;
        bool result     = value.toBool();
        QCOMPARE(result, true);
    }
    {
        Variant value   = 5;
        int result      = value.toInt();
        QCOMPARE(result, 5);
    }
    {
        Variant value   = 5.0f;
        float result    = value.toFloat();
        QCOMPARE(result, 5.0);
    }
    {
        Variant value   = "5";
        string result   = value.toString();
        QCOMPARE(result.c_str(), "5");
    }
    {
        Variant value   = string("5");
        string result   = value.toString();
        QCOMPARE(result.c_str(), "5");
    }
}

void Set_Get_Advanced_Check() {
    {
        Vector2 vector(1.0f, 2.0f);
        Variant value   = vector;
        QCOMPARE(value.toVector2(), vector);
    }
    {
        Vector3 vector(1.0f, 2.0f, 3.0f);
        Variant value   = vector;
        QCOMPARE(value.toVector3(), vector);
    }
    {
        Vector4 vector(1.0f, 2.0f, 3.0f, 4.0f);
        Variant value   = vector;
        QCOMPARE(value.toVector4(), vector);
    }
    {
        Quaternion quaternion(Vector3(1.0f, 2.0f, 3.0f), 4.0f);
        Variant value   = quaternion;
        QCOMPARE(value.toQuaternion(), quaternion);
    }
    {
        Matrix3 matrix  = Matrix3();
        Variant value   = matrix;
        QCOMPARE(value.toMatrix3(), matrix);
    }
    {
        Matrix4 matrix;
        Variant value   = matrix;
        QCOMPARE(value.toMatrix4(), matrix);
    }
}

void Convert_Bool_to_Int_Float_String() {
    {
        Variant value   = true;
        QVariant result = true;
        QCOMPARE(value.toInt(),     result.toInt());
        QCOMPARE(value.toFloat(),   result.toFloat());
        QCOMPARE(value.toString().c_str(),  result.toString().toStdString().c_str());
    }
}

void Convert_Int_to_Bool_Float_String() {
    {
        Variant value   = 5;
        QVariant result = 5;
        QCOMPARE(value.toBool(),    result.toBool());
        QCOMPARE(value.toFloat(),   result.toFloat());
        QCOMPARE(value.toString().c_str(),  result.toString().toStdString().c_str());
    }
}

void Convert_Float_to_Bool_Int_String() {
    {
        Variant value   = 0.6087612509727478f; // 6.4f
        QVariant result = 0.6087612509727478f;
        QCOMPARE(value.toBool(),    result.toBool());
        QCOMPARE(value.toInt(),     result.toInt());
        QCOMPARE(value.toString().c_str(),  "0.608761");
    }
    {
        Variant value   = 7.5f;
        QVariant result = 7.5f;
        QCOMPARE(value.toInt(),     result.toInt());
    }
}

void Convert_String_to_Bool_Int_Float() {
    {
        Variant value   = "true";
        QVariant result = "true";
        QCOMPARE(value.toBool(),    result.toBool());
    }
    {
        Variant value   = "8.4";
        QVariant result = "8.4";
        QCOMPARE(value.toBool(),    result.toBool());
        QCOMPARE(value.toInt(),     8);
        QCOMPARE(value.toFloat(),   result.toFloat());
    }
    {
        Variant value   = "9.6";
        QCOMPARE(value.toInt(),     9);
    }
}

void Compare_Variants_Basic() {
    {
        Variant value1(true);
        Variant value2(false);

        QCOMPARE((value1 == value1),    true);
        QCOMPARE((value1 == value2),    false);
    }
    {
        Variant value1(0);
        Variant value2(1);

        QCOMPARE((value1 == value1),    true);
        QCOMPARE((value1 == value2),    false);
    }
    {
        Variant value1(0.0f);
        Variant value2(1.0f);

        QCOMPARE((value1 == value1),    true);
        QCOMPARE((value1 == value2),    false);
    }
    {
        Variant value1("test1");
        Variant value2("test2");

        QCOMPARE((value1 == value1),    true);
        QCOMPARE((value1 == value2),    false);
    }
}

void Compare_Variants_Advanced() {
    {
        VariantList value1;
        value1.push_back("Test");
        value1.push_back(true);
        VariantList value2;
        value2.push_back("Test");
        value2.push_back(false);

        QCOMPARE((Variant(value1) == Variant(value1)),    true);
        QCOMPARE((Variant(value1) == Variant(value2)),    false);
    }
    {
        VariantMap value1;
        value1["NAME"]  = "Test";
        value1["VALUE"] = true;
        VariantMap value2;
        value2["NAME"]  = "Test";
        value2["VALUE"] = false;

        QCOMPARE((Variant(value1) == Variant(value1)),    true);
        QCOMPARE((Variant(value1) == Variant(value2)),    false);
    }
}

} REGISTER(VariantTest)

#include "tst_variant.moc"
