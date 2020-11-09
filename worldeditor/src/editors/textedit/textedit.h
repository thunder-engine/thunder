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
    TextEdit(DocumentModel *document);
    ~TextEdit();

signals:
    void templateUpdate();

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
    void loadAsset(IConverterSettings *settings) override;

    void closeEvent(QCloseEvent *event) override;
    bool isModified() const override;
    void setModified(bool flag) override;

    void saveAsset(const QString &path = QString()) override;

    QStringList assetTypes() const override;

    QFileInfo m_fileInfo;

    Ui::TextEdit *ui;

    DocumentModel *m_pDocument;
};

#endif // TEXTEDIT_H
