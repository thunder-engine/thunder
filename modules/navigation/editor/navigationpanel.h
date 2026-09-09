#ifndef NAVIGATIONPANEL_H
#define NAVIGATIONPANEL_H

#include <editor/editorgadget.h>

namespace Ui {
    class NavigationPanel;
}

class NavigationPanel : public EditorGadget {
    Q_OBJECT

public:
    explicit NavigationPanel(QWidget *parent = nullptr);
    ~NavigationPanel();

private:
    void onUpdated() override {}

    void onSelectionChanged() override {}
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override {}

private:
    Ui::NavigationPanel *ui;

};

#endif // NAVIGATIONPANEL_H
