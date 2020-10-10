#include "tst_common.h"

QObjectList TestRunnder::m_List;

int main(int argc, char *argv[]) {
    return TestRunnder::runAllTests(argc, argv);
}
