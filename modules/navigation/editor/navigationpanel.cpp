#include "navigationpanel.h"
#include "ui_navigationpanel.h"

#include <editor.h>
#include <editor/projectsettings.h>

NavigationPanel::NavigationPanel(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::NavigationPanel) {
    ui->setupUi(this);

    VariantMap map = Editor::project()->property("navigation").toMap();
    auto it = map.find("navmesh");
    if(it != map.end()) {
}

NavigationPanel::~NavigationPanel() {
    delete ui;
}
