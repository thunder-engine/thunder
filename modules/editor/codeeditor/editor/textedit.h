#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QFileInfo>

#include <editor/asseteditor.h>

namespace Ui {
    class TextEdit;
}

class TextEdit : public AssetEditor {
    Q_OBJECT

public:
    TextEdit();
    ~TextEdit();

private slots:
    void onCursorPositionChanged();
    void onTextChanged();

    void on_actionFind_triggered();

    void on_pushClose_clicked();

    void on_lineFind_textChanged(const QString &arg1);

    void on_pushPrevious_clicked();

    void on_pushNext_clicked();

    void on_pushReplace_clicked();

    void on_pushReplaceFind_clicked();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path = QString()) override;

    bool isSingleInstance() const override { return false; }

    AssetEditor *createInstance() override { return new TextEdit; }

    void changeEvent(QEvent *event) override;

    bool isModified() const override;
    void setModified(bool flag) override;

    QStringList suffixes() const override;

    Ui::TextEdit *ui;
};

#endif // TEXTEDIT_H
