#include "codeeditor.h"

#include <definition.h>
#include <foldingregion.h>
#include <syntaxhighlighter.h>
#include <theme.h>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QPainter>
#include <QPalette>

#include <QTextBlock>

#include "settingsmanager.h"

const QString gFont("Editors/Text_Editor/Font/Font_Name");
const QString gZoom("Editors/Text_Editor/Font/Zoom");

const QString gLineNumbers("Editors/Text_Editor/Display/Line_Numbers");
const QString gFoldingMarkers("Editors/Text_Editor/Display/Folding_Markers");
const QString gWhitespaces("Editors/Text_Editor/Display/Whitespaces");

const QString gSpaces("Editors/Text_Editor/Indents/Use_Spaces");
const QString gTabSize("Editors/Text_Editor/Indents/Tab_Size");

CodeEditor::CodeEditor(QWidget *parent) :
        QPlainTextEdit(parent),
        m_pHighlighter(new KSyntaxHighlighting::SyntaxHighlighter(document())),
        m_pSideBar(new CodeEditorSidebar(this)),
        m_SpaceTabs(true),
        m_SpaceIndent(4),
        m_BlockSelection(false),
        m_BlockPosition(0),
        m_ColumnPosition(0),
        m_BlockAnchor(0),
        m_ColumnAnchor(0),
        m_DisplayLineNumbers(true),
        m_DisplayFoldingMarkers(true),
        m_FirstTime(true) {

    setLineWrapMode(QPlainTextEdit::NoWrap);

    setTheme(m_Repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme));

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateSidebarGeometry);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateSidebarArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    QTextOption option = document()->defaultTextOption();
    option.setFlags(option.flags() | QTextOption::AddSpaceForLineAndParagraphSeparators);
    document()->setDefaultTextOption(option);

    SettingsManager *settings = SettingsManager::instance();
    settings->registerProperty(qPrintable(gFont), QVariant::fromValue(QFont("Source Code Pro", 10)));
    settings->registerProperty(qPrintable(gZoom), QVariant::fromValue(100));

    settings->registerProperty(qPrintable(gLineNumbers), QVariant::fromValue(true));
    settings->registerProperty(qPrintable(gFoldingMarkers), QVariant::fromValue(true));
    settings->registerProperty(qPrintable(gWhitespaces), QVariant::fromValue(false));

    settings->registerProperty(qPrintable(gSpaces), QVariant::fromValue(true));
    settings->registerProperty(qPrintable(gTabSize), QVariant::fromValue(4));

    connect(settings, &SettingsManager::updated, this, &CodeEditor::onApplySettings);

    onApplySettings();
}

CodeEditor::~CodeEditor() {
    delete m_pHighlighter;
    delete m_pSideBar;
}

void CodeEditor::openFile(const QString &fileName) {
    m_FileName = fileName;
    QFile fp(m_FileName);
    if (!fp.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << m_FileName << ":" << fp.errorString();
        return;
    }
    clear();

    const auto def = m_Repository.definitionForFileName(m_FileName);
    m_pHighlighter->setDefinition(def);

    setPlainText(QString::fromUtf8(fp.readAll()));
}

void CodeEditor::saveFile() {
    QFile fp(m_FileName);
    if (!fp.open(QFile::WriteOnly)) {
        qWarning() << "Failed to open" << m_FileName << ":" << fp.errorString();
        return;
    }

    fp.write(toPlainText().toUtf8());
    fp.close();

    document()->setModified(false);
}

void CodeEditor::setSpaceTabs(bool enable, uint32_t indent) {
    m_SpaceTabs = enable;
    m_SpaceIndent = indent;
}

void CodeEditor::displayLineNumbers(bool visible) {
    m_DisplayLineNumbers = visible;
    updateSidebarGeometry();
}

void CodeEditor::displayFoldingMarkers(bool visible) {
    m_DisplayFoldingMarkers = visible;
    updateSidebarGeometry();
}

void CodeEditor::decorateWhitespaces(bool value) {
    QTextOption option = document()->defaultTextOption();
    if(value) {
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    } else {
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
    }
    document()->setDefaultTextOption(option);
}

void CodeEditor::highlightBlock(const QString &text) {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QColor color(Qt::lightGray);

    QTextCursor cursor(document());
    cursor = document()->find(text, cursor);

    while(!cursor.isNull()) {
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(QColor(m_pHighlighter->theme().editorColor(KSyntaxHighlighting::Theme::SearchHighlight)));
        extra.cursor = cursor;
        extraSelections.append(extra);

        cursor = document()->find(text, cursor);
    }
    setExtraSelections(extraSelections);
}

bool CodeEditor::findString(const QString &string, bool reverse, bool casesens, bool words)  {
    QTextDocument::FindFlags flag;
    if(reverse) {
        flag |= QTextDocument::FindBackward;
    }
    if(casesens) {
        flag |= QTextDocument::FindCaseSensitively;
    }
    if(words) {
        flag |= QTextDocument::FindWholeWords;
    }

    QTextCursor cursor = textCursor();
    QTextCursor cursorSaved = cursor;

    if(!find(string, flag)) {
        cursor.movePosition(reverse ? QTextCursor::End : QTextCursor::Start);

        setTextCursor(cursor);

        if(!find(string, flag)) {
            setTextCursor(cursorSaved);
        }
        return false;
    }
    return true;
}

void CodeEditor::replaceSelected(const QString &string) {
    QTextCursor cursor = textCursor();

    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    if(!cursor.hasSelection()) {
        return;
    }

    cursor.setPosition(end, QTextCursor::KeepAnchor);
    QTextBlock endBlock = cursor.block();

    cursor.setPosition(start, QTextCursor::KeepAnchor);
    QTextBlock block = cursor.block();

    for(; block.isValid() && !(endBlock < block); block = block.next()) {
        if (!block.isValid()) {
            continue;
        }

        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.clearSelection();
        cursor.insertText(string);
        cursor.movePosition(QTextCursor::NextBlock);
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event) {
    auto menu = createStandardContextMenu(event->pos());

    menu->exec(event->globalPos());
    delete menu;
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    updateSidebarGeometry();
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    QTextCursor cursor = textCursor();
    if(event == QKeySequence::InsertParagraphSeparator) {
        cursor.beginEditBlock();
        int32_t indentRemain = firstNonIndent(cursor.block().text());
        if(m_pHighlighter->startsFoldingRegion(cursor.block())) {
            indentRemain += (m_SpaceTabs) ? m_SpaceIndent : 1;
        }
        cursor.insertBlock();

        QString text;
        text.fill((m_SpaceTabs) ? ' ' : '\t', indentRemain);
        cursor.insertText(text);
        cursor.endEditBlock();
        event->accept();

        return;
    }
    switch(event->key()) {
    case Qt::Key_Tab: {
        if(m_SpaceTabs) {
            int32_t indentRemain = m_SpaceIndent - (cursor.positionInBlock() % m_SpaceIndent);

            QString text;
            text.fill(' ', indentRemain);
            cursor.insertText(text);

            event->accept();
            return;
        }
    } break;
    case Qt::Key_Insert: {
        if(event->modifiers() == Qt::NoModifier) {
            setOverwriteMode(!overwriteMode());
            event->accept();
            return;
        }
    } break;
    default: break;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::mousePressEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        if(event->modifiers() & Qt::AltModifier) {
            QTextCursor cur = cursorForPosition(event->pos());
            int col = column(cur.block().text(), cur.positionInBlock());
            if (cur.positionInBlock() == cur.block().length() - 1) {
                col += (event->pos().x() - cursorRect(cur).center().x()) / QFontMetricsF(font()).width(QLatin1Char(' '));
            }

            int block = cur.blockNumber();
            if(block == blockCount() - 1) {
                block += (event->pos().y() - cursorRect(cur).center().y()) / QFontMetricsF(font()).lineSpacing();
            }
            if(m_BlockSelection) {
                m_BlockPosition = block;
                m_ColumnPosition = col;

                doSetTextCursor(cursor());
                viewport()->update();
            } else {
                enableBlockSelection(block, col, block, col);
            }
        } else {
            if(m_BlockSelection) {
                disableBlockSelection();
            }
        }
    }
    QPlainTextEdit::mousePressEvent(event);
}

void CodeEditor::mouseReleaseEvent(QMouseEvent *event) {
    QPlainTextEdit::mouseReleaseEvent(event);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event) {
    QPlainTextEdit::mouseMoveEvent(event);

    if(event->buttons() & Qt::LeftButton) {
        if(event->modifiers() & Qt::AltModifier) {
            if(m_BlockSelection) {
                QTextCursor cur = textCursor();
                int32_t col = column(cur.block().text(), cur.positionInBlock());
                if(cur.positionInBlock() == cur.block().length() - 1) {
                    col += (event->pos().x() - cursorRect().center().x()) / QFontMetricsF(font()).width(QLatin1Char(' '));
                }

                m_BlockPosition = cur.blockNumber();
                m_ColumnPosition = col;

                doSetTextCursor(cursor());
                viewport()->update();
            } else {
                if(textCursor().hasSelection()) {
                    QTextCursor cursor = textCursor();

                    QTextBlock positionTextBlock = cursor.block();
                    int32_t positionBlock = positionTextBlock.blockNumber();
                    int32_t positionColumn = column(positionTextBlock.text(), cursor.position() - positionTextBlock.position());

                    const QTextDocument *document = cursor.document();
                    QTextBlock anchorTextBlock = document->findBlock(cursor.anchor());
                    int32_t anchorBlock = anchorTextBlock.blockNumber();
                    int32_t anchorColumn = column(anchorTextBlock.text(), cursor.anchor() - anchorTextBlock.position());

                    enableBlockSelection(positionBlock, anchorColumn, anchorBlock, positionColumn);
                } else {
                    const QTextCursor &cursor = cursorForPosition(event->pos());
                    int32_t col = column(cursor.block().text(), cursor.positionInBlock());
                    if(cursor.positionInBlock() == cursor.block().length() - 1) {
                        col += (event->pos().x() - cursorRect().center().x()) / QFontMetricsF(font()).width(QLatin1Char(' '));
                    }

                    int32_t block = cursor.blockNumber();
                    if(block == blockCount() - 1) {
                        block += (event->pos().y() - cursorRect().center().y()) / QFontMetricsF(font()).lineSpacing();
                    }
                    enableBlockSelection(block, col, block, col);
                }
            }
        }
    }
}

void CodeEditor::setTheme(const KSyntaxHighlighting::Theme &theme) {
    auto pal = qApp->palette();
    if (theme.isValid()) {
        pal.setColor(QPalette::Base, theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
        pal.setColor(QPalette::Text, theme.textColor(KSyntaxHighlighting::Theme::Normal));
        pal.setColor(QPalette::Highlight, theme.editorColor(KSyntaxHighlighting::Theme::TextSelection));
    }
    setPalette(pal);

    m_pHighlighter->setTheme(theme);
    m_pHighlighter->rehighlight();
    highlightCurrentLine();
}

int CodeEditor::sidebarWidth() const {
    int digits = 1;
    auto count = blockCount();
    while (count >= 10) {
        ++digits;
        count /= 10;
    }

    return 4 + fontMetrics().width(QLatin1Char('9')) * digits + fontMetrics().lineSpacing();
}

void CodeEditor::sidebarPaintEvent(QPaintEvent *event) {
    QPainter painter(m_pSideBar);
    painter.fillRect(event->rect(), m_pHighlighter->theme().editorColor(KSyntaxHighlighting::Theme::IconBorder));

    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    const int currentBlockNumber = textCursor().blockNumber();

    const auto foldingMarkerSize = fontMetrics().lineSpacing();

    QFont f = font();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (m_DisplayLineNumbers && block.isVisible() && bottom >= event->rect().top()) {
            const auto number = QString::number(blockNumber + 1);
            bool current = (blockNumber == currentBlockNumber);
            painter.setPen(m_pHighlighter->theme().editorColor(
                current ? KSyntaxHighlighting::Theme::CurrentLineNumber
                        : KSyntaxHighlighting::Theme::LineNumbers));

            f.setBold(current);
            painter.setFont(f);
            painter.drawText(0, top, m_pSideBar->width() - 2 - foldingMarkerSize, fontMetrics().height(), Qt::AlignRight, number);
        }

        // folding marker
        if (m_DisplayFoldingMarkers && block.isVisible() && isFoldable(block)) {
            QPolygonF polygon;
            if (isFolded(block)) {
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.25);
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.75);
                polygon << QPointF(foldingMarkerSize * 0.8, foldingMarkerSize * 0.5);
            } else {
                polygon << QPointF(foldingMarkerSize * 0.25, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.75, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.5, foldingMarkerSize * 0.8);
            }
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(m_pHighlighter->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding)));
            painter.translate(m_pSideBar->width() - foldingMarkerSize, top);
            painter.drawPolygon(polygon);
            painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::updateSidebarGeometry() {
    setViewportMargins(sidebarWidth(), 0, 0, 0);
    const auto r = contentsRect();
    m_pSideBar->setGeometry(QRect(r.left(), r.top(), sidebarWidth(), r.height()));
    m_pSideBar->repaint();
}

void CodeEditor::updateSidebarArea(const QRect& rect, int dy) {
    if (dy) {
        m_pSideBar->scroll(0, dy);
    } else {
        m_pSideBar->update(0, rect.y(), m_pSideBar->width(), rect.height());
    }
}

void CodeEditor::highlightCurrentLine() {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(m_pHighlighter->theme().editorColor(KSyntaxHighlighting::Theme::CurrentLine)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

QTextBlock CodeEditor::blockAtPosition(int y) const {
    auto block = firstVisibleBlock();
    if (!block.isValid()) {
        return QTextBlock();
    }

    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    do {
        if (top <= y && y <= bottom) {
            return block;
        }
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    } while (block.isValid());
    return QTextBlock();
}

bool CodeEditor::isFoldable(const QTextBlock &block) const {
    return m_pHighlighter->startsFoldingRegion(block);
}

bool CodeEditor::isFolded(const QTextBlock &block) const {
    if (!block.isValid()) {
        return false;
    }
    const auto nextBlock = block.next();
    if (!nextBlock.isValid()) {
        return false;
    }
    return !nextBlock.isVisible();
}

void CodeEditor::toggleFold(const QTextBlock &startBlock) {
    if(m_DisplayFoldingMarkers) {
        auto endBlock = m_pHighlighter->findFoldingRegionEnd(startBlock).next();

        endBlock = endBlock.previous();

        if(isFolded(startBlock)) {
            // unfold
            auto block = startBlock.next();
            while (block.isValid() && !block.isVisible()) {
                block.setVisible(true);
                block.setLineCount(block.layout()->lineCount());
                block = block.next();
            }
        } else {
            // fold
            auto block = startBlock.next();
            while(block.isValid() && block != endBlock) {
                block.setVisible(false);
                block.setLineCount(0);
                block = block.next();
            }
        }
        document()->markContentsDirty(startBlock.position(), endBlock.position() - startBlock.position() + 1);

        emit document()->documentLayout()->documentSizeChanged(document()->documentLayout()->documentSize());
    }
}

int32_t CodeEditor::column(const QString &text, int32_t pos) const {
    int result = 0;
    for(int i = 0; i < pos; i++) {
        if(text.at(i) == QLatin1Char('\t')) {
            result -= (result % m_SpaceIndent) + m_SpaceIndent;
        } else {
            result++;
        }
    }
    return result;
}

int32_t CodeEditor::columnPosition(const QString &text, int column) const {
    int32_t current = 0;
    int32_t result = 0;
    int32_t textSize = text.size();
    while((result < textSize) && current < column) {
        if(result < textSize && text.at(result) == QLatin1Char('\t')) {
            current -= (current % m_SpaceIndent) + m_SpaceIndent;
        } else {
            current++;
        }
        result++;
    }
    return result;
}

QTextCursor CodeEditor::cursor() const {
    int32_t selectionAnchorColumn;
    int32_t selectionPositionColumn;

    int32_t firstBlock = qMin(m_BlockPosition, m_BlockAnchor);

    int32_t firstColumn = qMin(m_ColumnPosition, m_ColumnAnchor);
    int32_t lastColumn = qMax(m_ColumnPosition, m_ColumnAnchor);

    if(m_BlockAnchor == m_BlockPosition || true) {
        selectionAnchorColumn = m_ColumnAnchor;
        selectionPositionColumn = m_ColumnPosition;
    } else if(m_BlockAnchor == firstBlock){
        selectionAnchorColumn = firstColumn;
        selectionPositionColumn = lastColumn;
    } else {
        selectionAnchorColumn = lastColumn;
        selectionPositionColumn = firstColumn;
    }

    QTextDocument *doc = document();
    QTextCursor cursor(doc);

    QTextBlock anchorTextBlock = doc->findBlockByNumber(m_BlockAnchor);
    int32_t anchorPosition = anchorTextBlock.position() + columnPosition(anchorTextBlock.text(), selectionAnchorColumn);

    QTextBlock positionTextBlock = doc->findBlockByNumber(m_BlockPosition);
    int32_t cursorPosition = positionTextBlock.position() + columnPosition(positionTextBlock.text(), selectionPositionColumn);

    cursor.setPosition(anchorPosition);
    cursor.setPosition(cursorPosition, QTextCursor::KeepAnchor);

    return cursor;
}

void CodeEditor::enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn) {
    m_BlockPosition = positionBlock;
    m_ColumnPosition = positionColumn;

    m_BlockAnchor = anchorBlock;
    m_ColumnAnchor = anchorColumn;

    m_BlockSelection = true;
    doSetTextCursor(cursor()); // true
    viewport()->update();
}

void CodeEditor::disableBlockSelection() {
    m_BlockSelection = false;

    m_BlockPosition = 0;
    m_ColumnPosition = 0;

    m_BlockAnchor = 0;
    m_ColumnAnchor = 0;

    viewport()->update();
}

void CodeEditor::doSetTextCursor(const QTextCursor &cursor) {
    QTextCursor c = cursor;
    c.setVisualNavigation(true);
    QPlainTextEdit::doSetTextCursor(c);
}

int32_t CodeEditor::firstNonIndent(const QString &text) const {
    int32_t i = 0;
    while (i < text.size()) {
        if(text.at(i) != ' ' && text.at(i) != '\t') {
            return i;
        }
        ++i;
    }
    return i;
}

void CodeEditor::onApplySettings() {
    SettingsManager *s = SettingsManager::instance();
    QFont font = s->property(qPrintable(gFont)).value<QFont>();
    font.setFixedPitch(true);
    setFont(font);

    int32_t range = 0.01f * font.pointSize() * s->property(qPrintable(gZoom)).toFloat() - font.pointSize();
    zoomIn(range);

    displayLineNumbers(s->property(qPrintable(gLineNumbers)).toBool());
    displayFoldingMarkers(s->property(qPrintable(gFoldingMarkers)).toBool());

    decorateWhitespaces(s->property(qPrintable(gWhitespaces)).toBool());

    setSpaceTabs(s->property(qPrintable(gSpaces)).toBool(), s->property(qPrintable(gTabSize)).toInt());
}
