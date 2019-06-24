#include "textedit.h"
#include "ui_textedit.h"

#include <QSettings>
#include <QStyledItemDelegate>
#include <QDebug>

#include "codeeditor.h"

#include "documentmodel.h"
#include "projectmanager.h"

#include "editors/contentbrowser/contenttree.h"

class ProjectItemDeligate : public QStyledItemDelegate  {
private:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        QVariant value  = index.data(Qt::DecorationRole);
        switch(value.type()) {
            case QVariant::Image: {
                QImage image    = value.value<QImage>();
                if(!image.isNull()) {
                    image  = image.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    option->icon    = QIcon(QPixmap::fromImage(image));
                    option->decorationSize = image.size();
                }
            } break;
            default: break;
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
        return QSize(20, 20);
    }
};

class DocumentItemDeligate : public QStyledItemDelegate  {
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyleOptionViewItem opt = option;
        if(index.column() == 1) {
            if(!(option.state & QStyle::State_MouseOver)) {
                // Hide button
            }
        }
        QStyledItemDelegate::paint(painter, opt, index);
    }
};

TextEdit::TextEdit(Engine *engine) :
        QMainWindow(nullptr),
        IAssetEditor(engine),
        ui(new Ui::TextEdit) {

    ui->setupUi(this);

    m_pContentProxy = new ContentTreeFilter(this);
    m_pContentProxy->setSourceModel(ContentTree::instance());
    m_pContentProxy->setContentTypes({IConverter::ContentCode});
    m_pContentProxy->sort(0);

    ui->treeView->setItemDelegate(new ProjectItemDeligate);
    ui->treeView->setModel(m_pContentProxy);

    ui->splitter->setStretchFactor(1, 2);

    m_pDocumentModel = new DocumentModel;
    ui->docView->setModel(m_pDocumentModel);
    ui->docView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->docView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->docView->header()->resizeSection(1, 10);
    ui->docView->setItemDelegate(new DocumentItemDeligate);

    m_pEditor = ui->label;
}

TextEdit::~TextEdit() {
    delete ui;
}

void TextEdit::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value("text.geometry").toByteArray());
    QVariant value  = settings.value("text.splitter");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void TextEdit::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("text.geometry", saveGeometry());
    settings.setValue("text.splitter", ui->splitter->saveState());
}

void TextEdit::loadAsset(IConverterSettings *settings) {
    hide();
    show();
    CodeEditor *editor = m_pDocumentModel->openFile(settings->source());
    editor->setParent(ui->verticalLayoutWidget);

    ui->textLayout->insertWidget(1, editor);
    m_pEditor->hide();
    m_pEditor = editor;
    m_pEditor->show();
    m_pEditor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    ui->treeView->expandAll();

    connect(editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
}

void TextEdit::onCursorPositionChanged() {
    QTextCursor cursor = static_cast<CodeEditor *>(sender())->textCursor();
    ui->lineLabel->setText(QString("Line: %1, Col: %2").arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1));
}

void TextEdit::on_actionSaveCurrent_triggered() {
    CodeEditor *editor = dynamic_cast<CodeEditor *>(m_pEditor);
    if(editor) {
        editor->saveFile();
    }
}

void TextEdit::on_actionSaveAll_triggered() {
    m_pDocumentModel->saveAll();
}

void TextEdit::on_actionFind_triggered() {

}

void TextEdit::on_treeView_doubleClicked(const QModelIndex &index) {
    CodeEditor *editor = m_pDocumentModel->openFile(ContentTree::instance()->path(m_pContentProxy->mapToSource(index)));
    editor->setParent(ui->verticalLayoutWidget);

    ui->textLayout->insertWidget(1, editor);
    m_pEditor->hide();
    m_pEditor = editor;
    m_pEditor->show();
    m_pEditor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    connect(editor, &CodeEditor::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged);
}

void TextEdit::on_docView_clicked(const QModelIndex &index) {
    CodeEditor *editor = m_pDocumentModel->openFile(index);
    if(index.column() == 0) {
        ui->textLayout->insertWidget(1, editor);
        m_pEditor->hide();
        m_pEditor = editor;
        m_pEditor->show();
    } else {
        ui->textLayout->insertWidget(1, ui->label);
        m_pEditor = ui->label;

        m_pDocumentModel->closeFile(index);

        QModelIndex next = m_pDocumentModel->index(index.row(), 0, index.parent());
        if(next.isValid()) {
            on_docView_clicked(next);
        } else if(index.row() > 0) {
            on_docView_clicked(m_pDocumentModel->index(index.row() - 1, 0, index.parent()));
        } else {
            m_pEditor->show();
        }
    }
}
