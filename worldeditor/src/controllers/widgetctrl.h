#ifndef WIDGETCTRL_H
#define WIDGETCTRL_H

#include "objectctrl.h"

class WidgetCtrl : public ObjectCtrl {
public:
    WidgetCtrl                          (Engine *engine);
/*
    static void                         callback_transform_select   (ARender *pRender, int mode, bool simple, void *parent);
    static void                         callback_transform_draw     (ARender *pRender, int mode, bool simple, void *parent);
*/
    void                                update                      ();
};

#endif // WIDGETCTRL_H
