#include "textedit.h"
#include "ui_textedit.h"

#include <QSettings>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QMessageBox>
#include <QDir>

#include <editor/assetconverter.h>

#include "textwidget.h"

TextEdit::TextEdit() :
        ui(new Ui::TextEdit) {

    ui->setupUi(this);

    ui->findWidget->setProperty("pannel", true);

    connect(ui->editor, &TextWidget::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    connect(ui->editor, &TextWidget::textChanged, this, &TextEdit::onTextChanged);

    ui->editor->addAction(ui->actionFind);

    on_pushClose_clicked();
}

TextEdit::~TextEdit() {
    disconnect(ui->editor, &TextWidget::textChanged, this, &TextEdit::onTextChanged);
    disconnect(ui->editor, &TextWidget::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
    delete ui;
}

bool TextEdit::isModified() const {
    return ui->editor->document()->isModified();
}

void TextEdit::setModified(bool flag) {
    ui->editor->document()->setModified(flag);
}

void TextEdit::loadAsset(AssetConverterSettings *settings) {
    if(!m_settings.contains(settings)) {
        AssetEditor::loadAsset(settings);

        ui->editor->openFile(settings->source());
        setWindowTitle(QFileInfo(settings->source()).fileName());
    }
}

void TextEdit::saveAsset(const QString &path) {
    ui->editor->saveFile(path);
    onTextChanged();
}

void TextEdit::loadData(const Variant &data, const QString &suffix) {
    ui->editor->loadDefinition(QString("data.%1").arg(suffix));
    ui->editor->setPlainText(data.toString().c_str());
    ui->editor->setReadOnly(true);
}

void TextEdit::onCursorPositionChanged() {
    QTextCursor cursor = ui->editor->textCursor();
    ui->lineLabel->setText(QString("Line: %1, Col: %2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1));
}

void TextEdit::onTextChanged() {
    QString title;
    if(!m_settings.empty()) {
        title = QFileInfo(m_settings.first()->source()).fileName();
    }
    if(ui->editor->document() && ui->editor->document()->isModified()) {
        title.append('*');
    }
    setWindowTitle(title);
}

QStringList TextEdit::suffixes() const {
    return {"cpp", "h", "as", "txt", "json", "html", "htm", "xml", "shader", "vert", "frag"};
}

void TextEdit::on_actionFind_triggered() {
    ui->findWidget->show();
    ui->lineFind->setFocus();
    ui->lineFind->selectAll();
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
