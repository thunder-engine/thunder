#ifndef NAVIGATIONPANEL_H
#define NAVIGATIONPANEL_H

#include <editor/editorgadget.h>

#include <navigationsystem.h>

namespace Ui {
    class NavigationPanel;
}

class NavigationPanel : public EditorGadget {
    Q_OBJECT

public:
    explicit NavigationPanel(QWidget *parent = nullptr);
    ~NavigationPanel();

private:
    void loadSettings();
    void saveSettings();
    void updateEditor();
    void updateAgentType();

private slots:
    void onTypeSelected(int row);
    void onAddType();
    void onRemoveType();

private:
    void onUpdated() override {}

    void onSelectionChanged() override {}
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override {}

private:
    Ui::NavigationPanel *ui;
    std::vector<AgentType> m_agentTypes;
    bool m_updating = false;

};

#endif // NAVIGATIONPANEL_H
