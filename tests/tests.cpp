#include "tst_variant.h"
#include "tst_serialization.h"
#include "tst_threadpool.h"
#include "tst_uri.h"
#include "tst_object.h"
#include "tst_objectsystem.h"
#include "tst_metaobject.h"
#include "tst_animation.h"
#include "tst_actor.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
    //return TestRunnder::runAllTests(argc, argv);
}
