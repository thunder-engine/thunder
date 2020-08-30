#include "textedit.h"
#include "ui_textedit.h"

#include <QSettings>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QMessageBox>

#include "codeeditor.h"

#include "converters/converter.h"

#include "editors/contentbrowser/contenttree.h"

TextEdit::TextEdit(Engine *engine) :
        QWidget(nullptr),
        IAssetEditor(engine),
        ui(new Ui::TextEdit) {

    ui->setupUi(this);

    ui->findWidget->setProperty("pannel", true);

    connect(ui->editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    connect(ui->editor, &CodeEditor::textChanged, this, &TextEdit::onTextChanged);

    ui->editor->addAction(ui->actionFind);
    ui->editor->addAction(ui->actionSaveCurrent);

    on_pushClose_clicked();
}

TextEdit::~TextEdit() {
    disconnect(ui->editor, &CodeEditor::textChanged, this, &TextEdit::onTextChanged);
    disconnect(ui->editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    delete ui;
}

void TextEdit::closeEvent(QCloseEvent *event) {
    if(!checkSave()) {
        event->ignore();
    }
}

void TextEdit::loadAsset(IConverterSettings *settings) {
    if(checkSave()) {
        ui->editor->openFile(settings->source());
        m_fileInfo = QFileInfo(settings->source());
        setWindowTitle(m_fileInfo.fileName());
    }
}

bool TextEdit::checkSave() {
    if(ui->editor->document()->isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The file has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return false;
        } else if(result == QMessageBox::Yes) {
            on_actionSaveCurrent_triggered();
        } else {
            ui->editor->document()->setModified(false);
        }
    }
    return true;
}

void TextEdit::onCursorPositionChanged() {
    QTextCursor cursor = ui->editor->textCursor();
    ui->lineLabel->setText(QString("Line: %1, Col: %2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1));
}

void TextEdit::onTextChanged() {
    QString title = m_fileInfo.fileName();
    if(ui->editor->document() && ui->editor->document()->isModified()) {
        title.append('*');
    }
    setWindowTitle(title);
}

void TextEdit::on_actionSaveCurrent_triggered() {
    ui->editor->saveFile();
}

void TextEdit::on_actionFind_triggered() {
    ui->findWidget->show();
    ui->lineFind->setFocus();
}

void TextEdit::on_pushClose_clicked() {
    ui->findWidget->hide();
}

void TextEdit::on_lineFind_textChanged(const QString &arg1) {
    ui->editor->highlightBlock(arg1);
}

void TextEdit::on_pushPrevious_clicked() {
    ui->editor->findString(ui->lineFind->text(), true);
}

void TextEdit::on_pushNext_clicked() {
    ui->editor->findString(ui->lineFind->text(), false);
}

void TextEdit::on_pushReplace_clicked() {
    ui->editor->replaceSelected(ui->lineReplace->text());
}

void TextEdit::on_pushReplaceFind_clicked() {
    on_pushReplace_clicked();
    on_pushNext_clicked();
}

void TextEdit::on_pushReplaceAll_clicked() {
    while(ui->editor->findString(ui->lineFind->text(), false)) {
        on_pushReplace_clicked();
    }
}
