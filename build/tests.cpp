//#include "UnitTest++/UnitTest++.h"

#include "tst_common.h"

QObjectList TestRunnder::m_List;

int main(int argc, char *argv[]) {
    ObjectSystem system;
    TestObject::registerClassFactory(&system);

    return TestRunnder::runAllTests(argc, argv);
}
