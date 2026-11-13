#ifndef CONDITIONEDIT_H
#define CONDITIONEDIT_H

#include <editor/legacypropertyedit.h>

namespace Ui {
    class LegacyConditionEdit;
}

class AnimationControllerGraph;

class LegacyConditionEdit : public LegacyPropertyEdit {
    Q_OBJECT

public:
    explicit LegacyConditionEdit(QWidget *parent = nullptr);
    ~LegacyConditionEdit();

private slots:
    void on_comboName_currentIndexChanged(int index);

    void on_comboRule_currentIndexChanged(int index);

    void on_lineEdit_editingFinished();

    void on_checkBox_toggled(bool checked);

private:
    void setObject(Object *object, const TString &property) override;

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    Ui::LegacyConditionEdit *ui;

    TString m_conditionName;

    Variant m_conditionValue;

    AnimationControllerGraph *m_graph = nullptr;

    int m_conditionRule = -1;

};

#endif // CONDITIONEDIT_H
