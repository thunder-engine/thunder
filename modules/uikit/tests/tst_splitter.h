#include "gtest/gtest.h"

#include "components/splitter.h"
#include "uikit.h"

namespace UikitSuite {

    class SplitterTest : public ::testing::Test {

    };

    TEST_F(SplitterTest, AddWidgets) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* actor = Engine::composeActor<Splitter>("Test");
        Splitter* splitter = actor->getComponent<Splitter>();
        if (splitter) {
            Actor* actorFrame1 = Engine::composeActor<Frame>("Frame1");
            Frame* frame1 = actorFrame1->getComponent<Frame>();

            Actor* actorFrame2 = Engine::composeActor<Frame>("Frame2");
            Frame* frame2 = actorFrame2->getComponent<Frame>();

            splitter->addWidget(frame1);
            splitter->addWidget(frame2);

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame1, splitter->widget(0));
            ASSERT_EQ(frame2, splitter->widget(1));
            ASSERT_EQ(nullptr, splitter->widget(2));
            ASSERT_EQ(nullptr, splitter->widget(-1));

            ASSERT_EQ(1, splitter->indexOf(frame2));
            ASSERT_EQ(-1, splitter->indexOf(nullptr));
        }
    }

    TEST_F(SplitterTest, InsertWidget) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* actor = Engine::composeActor<Splitter>("Test");
        Splitter* splitter = actor->getComponent<Splitter>();
        if (splitter) {
            Actor* actorFrame1 = Engine::composeActor<Frame>("Frame1");
            Frame* frame1 = actorFrame1->getComponent<Frame>();

            Actor* actorFrame2 = Engine::composeActor<Frame>("Frame2");
            Frame* frame2 = actorFrame2->getComponent<Frame>();

            Actor* actorFrame3 = Engine::composeActor<Frame>("Frame3");
            Frame* frame3 = actorFrame3->getComponent<Frame>();

            splitter->addWidget(frame1);
            splitter->addWidget(frame3);
            splitter->insertWidget(1, frame2);

            ASSERT_EQ(3, splitter->count());
            ASSERT_EQ(frame2, splitter->widget(1));
            ASSERT_EQ(frame3, splitter->widget(2));
        }
    }

    TEST_F(SplitterTest, ReplaceWidget) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* actor = Engine::composeActor<Splitter>("Test");
        Splitter* splitter = actor->getComponent<Splitter>();
        if (splitter) {
            Actor* actorFrame1 = Engine::composeActor<Frame>("Frame1");
            Frame* frame1 = actorFrame1->getComponent<Frame>();

            Actor* actorFrame2 = Engine::composeActor<Frame>("Frame2");
            Frame* frame2 = actorFrame2->getComponent<Frame>();

            Actor* actorFrame3 = Engine::composeActor<Frame>("Frame3");
            Frame* frame3 = actorFrame3->getComponent<Frame>();

            splitter->addWidget(frame1);
            splitter->addWidget(frame3);
            Widget* widget = splitter->replaceWidget(1, frame2);

            ASSERT_EQ(frame3, widget);
            actorFrame3->setParent(nullptr);

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame2, splitter->widget(1));
        }
    }

    TEST_F(SplitterTest, AddChild) {
        Engine engine("");
        engine.addModule(new UiKit(&engine));

        Actor* actor = Engine::composeActor<Splitter>("Test");
        Splitter* splitter = actor->getComponent<Splitter>();
        if (splitter) {
            Actor* actorFrame1 = Engine::composeActor<Frame>("Frame1", actor);
            Frame* frame1 = actorFrame1->getComponent<Frame>();

            Actor* actorFrame2 = Engine::composeActor<Frame>("Frame2", actor);
            Frame* frame2 = actorFrame2->getComponent<Frame>();

            ASSERT_EQ(2, splitter->count());
            ASSERT_EQ(frame1, splitter->widget(0));
            ASSERT_EQ(frame2, splitter->widget(1));
        }
    }
}
