#include "tst_variant.h"
#include "tst_serialization.h"
#include "tst_threadpool.h"
#include "tst_url.h"
#include "tst_object.h"
#include "tst_objectsystem.h"
#include "tst_metaobject.h"
#include "tst_animation.h"
#include "tst_actor.h"
#include "tst_animationtrack.h"
#include "tst_animator.h"
//#include "tst_network.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
