#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>
#include <QFileInfo>

#include "editors/scenecomposer/documentmodel.h"

namespace Ui {
    class TextEdit;
}

class ContentTreeFilter;
class DocumentModel;

class TextEdit : public QWidget, public IAssetEditor {
    Q_OBJECT

public:
    explicit TextEdit(Engine *engine);
    ~TextEdit();

    void loadAsset(IConverterSettings *settings) override;

signals:
    void templateUpdate();

private slots:
    void onCursorPositionChanged();
    void onTextChanged();

    void on_actionSaveCurrent_triggered();

    void on_actionFind_triggered();

    void on_pushClose_clicked();

    void on_lineFind_textChanged(const QString &arg1);

    void on_pushPrevious_clicked();

    void on_pushNext_clicked();

    void on_pushReplace_clicked();

    void on_pushReplaceFind_clicked();

    void on_pushReplaceAll_clicked();

private:
    void closeEvent(QCloseEvent *event) override;

    bool checkSave();

    QFileInfo m_fileInfo;

    Ui::TextEdit *ui;
};

#endif // TEXTEDIT_H
