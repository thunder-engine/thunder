#include "widgetctrl.h"

#include <engine.h>

#include <components/actor.h>

WidgetCtrl::WidgetCtrl() :
        ObjectCtrl(nullptr) {
}

/// \todo: Rework Controller
/*
void WidgetCtrl::callback_transform_select(ARender *pRender, int mode, bool simple, void *parent) {
    WidgetCtrl *pControl   = (WidgetCtrl *)parent;

    if(pControl->get_object()) {
        Vector2 screen    = pRender->screen();

        pRender->object_retrive_id_begin(1);
        ARender::ortho_begin(screen.x, screen.y);

        for(uint32_t i = 0; i < pControl->mList.size(); i++) {
            AWidget *pWidget    = (AWidget *)pControl->mList[i]->object;
            Vector3 pos   = pWidget->absolute();

            float w, h;
            pWidget->size(w, h);

            glPushMatrix();

            Vector3 color;
            glTranslatef(pos.x, screen.y - pos.y - h, 0.0f);
            pRender->select_begin(WidgetCtrl::AXIS_Z, color);
            glColor3fv(color.v);
            ARender::draw_quad(w, h);

            pRender->select_begin(WidgetCtrl::AXIS_Y, color);
            glColor3fv(color.v);
            ARender::draw_quad(10.0f, 10.0f);

            glTranslatef(w - 10.0f, 0.0f, 0.0f);
            pRender->select_begin(WidgetCtrl::AXIS_X | WidgetCtrl::AXIS_Y, color);
            glColor3fv(color.v);
            ARender::draw_quad(10.0f, 10.0f);

            glTranslatef(0.0f, h - 10.0f, 0.0f);
            pRender->select_begin(WidgetCtrl::AXIS_X, color);
            glColor3fv(color.v);
            ARender::draw_quad(10.0f, 10.0f);

            pRender->select_end();
            glColor3f(1.0f, 1.0f, 1.0f);

            glPopMatrix();
        }

        ARender::ortho_end();
        pControl->set_axis( pRender->object_retrive_id_end() );
    }

}
*/
/*
void WidgetCtrl::callback_transform_draw(ARender *pRender, int mode, bool simple, void *parent) {
    WidgetCtrl *pControl    = (WidgetCtrl *)parent;

    if(pControl->get_object()) {
        Vector2 screen    = pRender->screen();
        ARender::ortho_begin(screen.x, screen.y);

        for(uint32_t i = 0; i < pControl->mList.size(); i++) {
            AWidget *pWidget    = (AWidget *)pControl->mList[i]->object;
            Vector3 pos   = pWidget->absolute();

            float w, h;
            pWidget->size(w, h);

            if(pControl->axis == WidgetCtrl::AXIS_Z)
                glColor3f(1.0, 1.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_LINE_LOOP);
                glVertex2d(pos.x,       screen.y - pos.y        );
                glVertex2d(pos.x + w,   screen.y - pos.y        );
                glVertex2d(pos.x + w,   screen.y - pos.y - h    );
                glVertex2d(pos.x,       screen.y - pos.y - h    );
            glEnd();

            glPushMatrix();

            glTranslatef(pos.x, screen.y - pos.y - h, 0.0f);
            if(pControl->axis == WidgetCtrl::AXIS_Y)
                glColor3f(1.0, 1.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);
            ARender::draw_quad(10.0f, 10.0f);

            glTranslatef(w - 10.0f, 0.0f, 0.0f);
            if(pControl->axis & WidgetCtrl::AXIS_X &&
               pControl->axis & WidgetCtrl::AXIS_Y)
                glColor3f(1.0, 1.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);
            ARender::draw_quad(10.0f, 10.0f);

            glTranslatef(0.0f, h - 10.0f, 0.0f);
            if(pControl->axis == WidgetCtrl::AXIS_X)
                glColor3f(1.0, 1.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);
            ARender::draw_quad(10.0f, 10.0f);

            glColor3f(1.0f, 1.0f, 1.0f);

            glPopMatrix();
        }
        ARender::ortho_end();
    }
}
*/
void WidgetCtrl::update() {
    if(mDrag) {
        Vector3 delta;//     = world - old;
/*
        if(mAxis == Transform::AXIS_Z) {
            for(const auto &it : mList) {
                Actor *actor   = dynamic_cast<Actor *>(it->object);
                //Vector3 *p    = ->get_pos();
                //*p                 += delta;
            }
        }

        if(mAxis & Transform::AXIS_X || mAxis & Transform::AXIS_Y) {
            //AWidget *pWidget        = (AWidget *)mList[0]->object;

            float w, h;
            pWidget->size(w, h);
            if(mAxis & WidgetCtrl::AXIS_X)
                w      += delta.x;
            if(mAxis & WidgetCtrl::AXIS_Y)
                h      += delta.y;

            if(w < 1.0f)
                w       = 1.0;
            if(h < 1.0f)
                h       = 1.0;

            pWidget->setSize(w, h);

        }
*/
    }
}

/*
        if(is_widget()) {
            this->pos       = *world;   // For position delta calculating
            select_vector::iterator it  = mList.begin();
            while(it != mList.end()) {
                select_data *data   = *it;
                data->position      = *data->object->get_pos();
                it++;
            }
//          this->axis      = AXIS_Z;
        }
*/
