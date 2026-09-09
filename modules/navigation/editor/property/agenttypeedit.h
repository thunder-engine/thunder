#ifndef AGENTTYPEEDIT_H
#define AGENTTYPEEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class AgentTypeEdit;
}

class AgentTypeEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit AgentTypeEdit(QWidget *parent = nullptr);
    ~AgentTypeEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    Ui::AgentTypeEdit *ui;
};

#endif // AGENTTYPEEDIT_H
