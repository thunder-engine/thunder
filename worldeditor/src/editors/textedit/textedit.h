#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>

#include "assetmanager.h"

namespace Ui {
    class TextEdit;
}

class ContentTreeFilter;
class DocumentModel;

class TextEdit : public QMainWindow, public IAssetEditor {
    Q_OBJECT

public:
    explicit TextEdit(Engine *engine);
    ~TextEdit();

    void readSettings();
    void writeSettings();

    void loadAsset(IConverterSettings *settings);

signals:
    void templateUpdate();

private slots:
    void onCursorPositionChanged();

    void on_actionSaveCurrent_triggered();
    void on_actionSaveAll_triggered();

    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_actionFind_triggered();

    void on_docView_clicked(const QModelIndex &index);

private:
    Ui::TextEdit *ui;

    ContentTreeFilter *m_pContentProxy;

    DocumentModel *m_pDocumentModel;

    QWidget *m_pEditor;
};

#endif // TEXTEDIT_H
