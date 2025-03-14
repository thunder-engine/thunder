#ifndef RHIWRAPPER_H
#define RHIWRAPPER_H

class QWindow;
class Viewport;

QWindow *createWindow(Viewport *viewport);

void makeCurrent();

#endif // RHIWRAPPER_H
