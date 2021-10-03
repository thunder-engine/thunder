#include "textedit.h"
#include "ui_textedit.h"

#include <QSettings>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QMessageBox>
#include <QDir>

#include <editor/assetconverter.h>

#include "codeeditor.h"

TextEdit::TextEdit() :
        ui(new Ui::TextEdit) {

    ui->setupUi(this);

    ui->findWidget->setProperty("pannel", true);

    connect(ui->editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    connect(ui->editor, &CodeEditor::textChanged, this, &TextEdit::onTextChanged);

    ui->editor->addAction(ui->actionFind);

    on_pushClose_clicked();
}

TextEdit::~TextEdit() {
    disconnect(ui->editor, &CodeEditor::textChanged, this, &TextEdit::onTextChanged);
    disconnect(ui->editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    delete ui;
}

bool TextEdit::isModified() const {
    return ui->editor->document()->isModified();
}

void TextEdit::setModified(bool flag) {
    ui->editor->document()->setModified(flag);
}

void TextEdit::loadAsset(AssetConverterSettings *settings) {
    m_pSettings = settings;
    ui->editor->openFile(m_pSettings->source());
    setWindowTitle(QFileInfo(m_pSettings->source()).fileName());
}

void TextEdit::saveAsset(const QString &path) {
    ui->editor->saveFile(path);
    onTextChanged();
}

void TextEdit::onCursorPositionChanged() {
    QTextCursor cursor = ui->editor->textCursor();
    ui->lineLabel->setText(QString("Line: %1, Col: %2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1));
}

void TextEdit::onTextChanged() {
    QString title = QFileInfo(m_pSettings->source()).fileName();
    if(ui->editor->document() && ui->editor->document()->isModified()) {
        title.append('*');
    }
    setWindowTitle(title);
}

QStringList TextEdit::suffixes() const {
    return {"cpp", "h", "as", "txt", "json", "html", "htm", "xml"};
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

void TextEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
