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
#include <QAbstractItemModel>
#include <QClipboard>
#include <QMimeData>

#include <QTextBlock>

#include "settingsmanager.h"
#include "assetmanager.h"

const QString gFont("Editors/Text_Editor/Font/Font_Name");
const QString gZoom("Editors/Text_Editor/Font/Zoom");

const QString gLineNumbers("Editors/Text_Editor/Display/Line_Numbers");
const QString gFoldingMarkers("Editors/Text_Editor/Display/Folding_Markers");
const QString gWhitespaces("Editors/Text_Editor/Display/Whitespaces");

const QString gSpaces("Editors/Text_Editor/Indents/Use_Spaces");
const QString gTabSize("Editors/Text_Editor/Indents/Tab_Size");

CodeEditor::CodeEditor(QWidget *parent) :
        QPlainTextEdit(parent),
        m_highlighter(new KSyntaxHighlighting::SyntaxHighlighter(document())),
        m_classModel(nullptr),
        m_sideBar(new CodeEditorSidebar(this)),
        m_spaceTabs(true),
        m_spaceIndent(4),
        m_blockSelection(false),
        m_blockPosition(0),
        m_columnPosition(0),
        m_blockAnchor(0),
        m_columnAnchor(0),
        m_displayLineNumbers(true),
        m_displayFoldingMarkers(true),
        m_cursorVisible(false) {

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_repository.addCustomSearchPath(":/Themes");
    setTheme(m_repository.theme("Thunder Dark"));

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

    startTimer(500);
}

CodeEditor::~CodeEditor() {
    delete m_highlighter;
    delete m_sideBar;
}

void CodeEditor::openFile(const QString &fileName) {
    m_fileName = fileName;
    QFile fp(m_fileName);
    if (!fp.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << m_fileName << ":" << fp.errorString();
        return;
    }
    clear();

    m_definition = m_repository.definitionForFileName(m_fileName);

    AssetManager::ClassMap map = AssetManager::instance()->classMaps();
    m_classModel = map.value(QFileInfo(m_fileName).suffix());
    if(m_classModel) {
        connect(m_classModel, &QAbstractItemModel::layoutChanged, this, &CodeEditor::onClassModelChanged);
        onClassModelChanged();
    }
    m_highlighter->setDefinition(m_definition);

    setPlainText(QString::fromUtf8(fp.readAll()));
}

void CodeEditor::saveFile(const QString &path) {
    if(!path.isEmpty()) {
        m_fileName = path;
    }
    QFile fp(m_fileName);
    if (!fp.open(QFile::WriteOnly)) {
        qWarning() << "Failed to open" << m_fileName << ":" << fp.errorString();
        return;
    }

    fp.write(toPlainText().toUtf8());
    fp.close();

    document()->setModified(false);
}

void CodeEditor::setSpaceTabs(bool enable, uint32_t indent) {
    m_spaceTabs = enable;
    m_spaceIndent = indent;
}

void CodeEditor::displayLineNumbers(bool visible) {
    m_displayLineNumbers = visible;
    updateSidebarGeometry();
}

void CodeEditor::displayFoldingMarkers(bool visible) {
    m_displayFoldingMarkers = visible;
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

    QTextCursor cursor(document());
    cursor = document()->find(text, cursor);

    while(!cursor.isNull()) {
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::SearchHighlight)));
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

void CodeEditor::reportIssue(int level, int line, int col, const QString &text) {

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
    if(isReadOnly()) {
        event->accept();

        return;
    }

    QTextCursor cursor = textCursor();

    if(event == QKeySequence::InsertParagraphSeparator) {
        cursor.beginEditBlock();

        int32_t indentRemain = firstNonIndent(cursor.block().text());
        if(m_highlighter->startsFoldingRegion(cursor.block())) {
            indentRemain += (m_spaceTabs) ? m_spaceIndent : 1;
        }

        cursor.insertBlock();

        QString text;
        text.fill((m_spaceTabs) ? ' ' : '\t', indentRemain);
        cursor.insertText(text);

        cursor.endEditBlock();

        event->accept();
        return;
    }

    if(m_blockSelection) {
        if(event == QKeySequence::Copy) {
            QApplication::clipboard()->setText(copyBlockSelection());

            event->accept();
            return;
        } else if(event == QKeySequence::Cut) {
            QApplication::clipboard()->setText(copyBlockSelection());
            removeBlockSelection();

            event->accept();
            return;
        } else if(event == QKeySequence::Delete || event->key() == Qt::Key_Backspace) {
            if(m_columnPosition == m_columnAnchor) {
                if(event == QKeySequence::Delete) {
                    ++m_columnPosition;
                } else if(m_columnPosition > 0) {
                    --m_columnPosition;
                }
            }
            removeBlockSelection();

            event->accept();
            return;
        } else if(event == QKeySequence::Paste) {
            removeBlockSelection();
            //paste();
        }
    }

    switch(event->key()) {
        case Qt::Key_Slash: {
            if(event->modifiers() == Qt::ControlModifier) {
                commentSelection();

                event->accept();
                return;
            }
        } break;
        case Qt::Key_Tab: {
            if(m_blockSelection && qMin(m_columnPosition, m_columnAnchor) != qMax(m_columnPosition, m_columnAnchor)) {
                removeBlockSelection();
            } else {
                indentSelection();
            }
            event->accept();
            return;
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

    if(m_blockSelection) {
        const QString text = event->text();
        if(!text.isEmpty() && (text.at(0).isPrint() || text.at(0) == QLatin1Char('\t'))) {
            insertIntoBlockSelection(text);

            return;
        }
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
            if(m_blockSelection) {
                m_blockPosition = block;
                m_columnPosition = col;

                doSetTextCursor(cursor(), true);
                viewport()->update();
            } else {
                enableBlockSelection(block, col, block, col);
            }
        } else {
            if(m_blockSelection) {
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
            if(m_blockSelection) {
                QTextCursor cur = textCursor();
                int32_t col = column(cur.block().text(), cur.positionInBlock());
                if(cur.positionInBlock() == cur.block().length() - 1) {
                    col += (event->pos().x() - cursorRect().center().x()) / QFontMetricsF(font()).width(QLatin1Char(' '));
                }

                m_blockPosition = cur.blockNumber();
                m_columnPosition = col;

                doSetTextCursor(cursor(), true);
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

void CodeEditor::paintEvent(QPaintEvent *event) {
    QPainter painter(viewport());

    QPointF offset(contentOffset());
    QRect er = event->rect();
    QRect viewportRect = viewport()->rect();
    bool editable = !isReadOnly();
    QTextBlock block = firstVisibleBlock();
    qreal maximumWidth = document()->documentLayout()->documentSize().width();

    painter.setBrushOrigin(offset);

    int maxX = offset.x() + qMax((qreal)viewportRect.width(), maximumWidth) - document()->documentMargin();
    er.setRight(qMin(er.right(), maxX));
    painter.setClipRect(er);

    QAbstractTextDocumentLayout::PaintContext context = getPaintContext();
    painter.setPen(context.palette.text().color());
    while(block.isValid()) {
        QRectF r = blockBoundingRect(block).translated(offset);
        QTextLayout *layout = block.layout();
        if(!block.isVisible()) {
            offset.ry() += r.height();
            block = block.next();
            continue;
        }
        if(r.bottom() >= er.top() && r.top() <= er.bottom()) {
            QTextBlockFormat blockFormat = block.blockFormat();

            QVector<QTextLayout::FormatRange> selections;

            int blpos = block.position();
            int bllen = block.length();

            setupSelections(block, blpos, bllen, selections);

            QRectF rect;
            paintBlockSelection(block, painter, offset, rect);

            bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard))
                               && context.cursorPosition >= blpos
                               && context.cursorPosition < blpos + bllen);
            bool drawCursorAsBlock = drawCursor && overwriteMode();
            if(drawCursorAsBlock) {
                if(context.cursorPosition == blpos + bllen - 1) {
                    drawCursorAsBlock = false;
                } else {
                    QTextLayout::FormatRange o;
                    o.start = context.cursorPosition - blpos;
                    o.length = 1;
                    o.format.setForeground(palette().base());
                    o.format.setBackground(palette().text());
                    selections.append(o);
                }
            }

            layout->draw(&painter, offset, selections, er);

            if(!m_blockSelection) {
                if((drawCursor && !drawCursorAsBlock) || (editable && context.cursorPosition < -1 && !layout->preeditAreaText().isEmpty())) {
                    int cpos = context.cursorPosition;
                    if(cpos < -1) {
                        cpos = layout->preeditAreaPosition() - (cpos + 2);
                    } else {
                        cpos -= blpos;
                    }
                    layout->drawCursor(&painter, offset, cpos, cursorWidth());
                }
            } else if(rect.isValid() && m_cursorVisible) {
                painter.fillRect(rect, palette().text());
            }
        }
        offset.ry() += r.height();
        if(offset.y() > viewportRect.height()) {
            break;
        }
        block = block.next();
    }
}

void CodeEditor::timerEvent(QTimerEvent *event) {
    m_cursorVisible = !m_cursorVisible;
    update();
}

void CodeEditor::commentSelection() {
    QTextCursor cursor = textCursor();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    bool hasSelection = cursor.hasSelection();

    QTextDocument *doc = document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    static const QString singleLine("//");
    static const QString multiLineBegin("/*");
    static const QString multiLineEnd("*/");

    static const bool hasMultiLineStyle = true;
    static const bool hasSingleLineStyle = true;

    bool doSingleLineStyleUncomment = true;
    bool doMultiLineStyleComment = false;
    bool doMultiLineStyleUncomment = false;
    bool anchorIsStart = (anchor == start);

    cursor.beginEditBlock();

    if(hasSelection && hasMultiLineStyle) {
        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
        const int multiLineStartLength = multiLineBegin.length();
        bool hasLeadingCharacters = !startText.left(startPos).trimmed().isEmpty();

        int pos = startPos - multiLineStartLength;
        if(startPos >= multiLineStartLength
            && startText.indexOf(multiLineBegin, pos) == pos) {
            startPos -= multiLineStartLength;
            start -= multiLineStartLength;
        }

        bool hasSelStart = startPos <= startText.length() - multiLineStartLength
            && startText.indexOf(multiLineBegin, startPos) == startPos;

        QString endText = endBlock.text();
        int endPos = end - endBlock.position();
        const int multiLineEndLength = multiLineEnd.length();
        bool hasTrailingCharacters =
                !endText.left(endPos).remove(singleLine).trimmed().isEmpty()
                && !endText.mid(endPos).trimmed().isEmpty();

        if(endPos <= endText.length() - multiLineEndLength && endText.indexOf(multiLineEnd, endPos) == endPos) {
            endPos += multiLineEndLength;
            end += multiLineEndLength;
        }

        pos = endPos - multiLineEndLength;
        bool hasSelEnd = endPos >= multiLineEndLength && endText.indexOf(multiLineEnd, pos) == pos;

        doMultiLineStyleUncomment = hasSelStart && hasSelEnd;
        doMultiLineStyleComment = !doMultiLineStyleUncomment && (hasLeadingCharacters || hasTrailingCharacters || !hasSingleLineStyle);

    } else if(!hasSelection && !hasSingleLineStyle) {
        QString text = startBlock.text().trimmed();
        doMultiLineStyleUncomment = text.startsWith(multiLineBegin) && text.endsWith(multiLineEnd);
        doMultiLineStyleComment = !doMultiLineStyleUncomment && !text.isEmpty();

        start = startBlock.position();
        end = endBlock.position() + endBlock.length() - 1;

        if(doMultiLineStyleUncomment) {
            int offset = 0;
            text = startBlock.text();
            const int length = text.length();
            while(offset < length && text.at(offset).isSpace()) {
                ++offset;
            }
            start += offset;
        }
    }

    if(doMultiLineStyleUncomment) {
        cursor.setPosition(end);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, multiLineEnd.length());
        cursor.removeSelectedText();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, multiLineBegin.length());
        cursor.removeSelectedText();
    } else if(doMultiLineStyleComment) {
        cursor.setPosition(end);
        cursor.insertText(multiLineEnd);
        cursor.setPosition(start);
        cursor.insertText(multiLineBegin);
    } else {
        endBlock = endBlock.next();

        doSingleLineStyleUncomment = true;
        for(QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text().trimmed();
            if(!text.isEmpty() && !text.startsWith(singleLine)) {
                doSingleLineStyleUncomment = false;
                break;
            }
        }

        const int singleLineLength = singleLine.length();
        for(QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            if(doSingleLineStyleUncomment) {
                QString text = block.text();

                int i = 0;
                while(i <= text.size() - singleLineLength) {
                    if(text.indexOf(singleLine, i) == i) {
                        cursor.setPosition(block.position() + i);
                        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, singleLineLength);
                        cursor.removeSelectedText();
                        break;
                    }
                    if(!text.at(i).isSpace()) {
                        break;
                    }
                    ++i;
                }
            } else {
                cursor.setPosition(block.position());
                cursor.insertText(singleLine);
            }
        }
    }

    cursor.endEditBlock();

    if(hasSelection && !doMultiLineStyleUncomment) {
        cursor = textCursor();
        if(!doMultiLineStyleComment)
            start = startBlock.position();
        int lastSelPos = anchorIsStart ? cursor.position() : cursor.anchor();
        if(anchorIsStart) {
            cursor.setPosition(start);
            cursor.setPosition(lastSelPos, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(lastSelPos);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
        }
        setTextCursor(cursor);
    }
}

void CodeEditor::indentSelection() {
    QTextCursor cur = textCursor();

    cur.beginEditBlock();

    int pos = cur.position();

    int col = m_blockSelection ? m_columnPosition : column(cur.block().text(), cur.positionInBlock());

    int anchor = cur.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    QTextBlock startBlock = document()->findBlock(start);
    QTextBlock endBlock = document()->findBlock(qMax(end - 1, 0)).next();

    const bool cursorAtBlockStart = (textCursor().position() == startBlock.position());
    const bool anchorAtBlockStart = (textCursor().anchor() == startBlock.position());
    const bool oneLinePartial = (startBlock.next() == endBlock)
                              && (start > startBlock.position() || end < endBlock.position() - 1);

    if (startBlock == endBlock) {
        endBlock = endBlock.next();
    }
    if (cur.hasSelection() && !m_blockSelection && !oneLinePartial) {
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            const QString text = block.text();
            int indentPosition = lineIndentPosition(text);

            int targetColumn = indentedColumn(column(text, indentPosition), true);
            cur.setPosition(block.position() + indentPosition);
            cur.insertText(indentationString(0, targetColumn, 0, block));
            cur.setPosition(block.position());
            cur.setPosition(block.position() + indentPosition, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
        }
        if (cursorAtBlockStart) {
            cur = textCursor();
            cur.setPosition(startBlock.position(), QTextCursor::KeepAnchor);
        } else if (anchorAtBlockStart) {
            cur = textCursor();
            cur.setPosition(startBlock.position(), QTextCursor::MoveAnchor);
            cur.setPosition(textCursor().position(), QTextCursor::KeepAnchor);
        }
    } else if (cur.hasSelection() && !m_blockSelection && oneLinePartial) {
        cur.removeSelectedText();
    } else {
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text();

            int blockColumn = column(text, text.size());
            if (blockColumn < col) {
                cur.setPosition(block.position() + text.size());
                cur.insertText(indentationString(blockColumn, col, 0, block));
                text = block.text();
            }

            int indentPosition = columnPosition(text, col, 0);
            int spaces = spacesLeftFromPosition(text, indentPosition);
            int startColumn = column(text, indentPosition - spaces);
            int targetColumn = indentedColumn(column(text, indentPosition), true);
            cur.setPosition(block.position() + indentPosition);
            cur.setPosition(block.position() + indentPosition - spaces, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
            cur.insertText(indentationString(startColumn, targetColumn, 0, block));
        }

        if(m_blockSelection) {
            end = cur.position();
            int offset = column(cur.block().text(), cur.positionInBlock()) - col;

            m_columnAnchor += offset;
            m_columnPosition += offset;

            cur.setPosition(start);
            cur.setPosition(end, QTextCursor::KeepAnchor);
        }
    }

    cur.endEditBlock();

    doSetTextCursor(cur, true);
/*
    if(cur.hasSelection()) { // Insert indents for a selected text
        for(QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            cur.setPosition(block.position());

            QString text;
            text.fill((m_spaceTabs) ? ' ' : '\t', (m_spaceTabs) ? m_spaceIndent : 1);
            cur.insertText(text);
        }
    } else { // Indent at cursor position
        if(m_spaceTabs) {
            int32_t indentRemain = m_spaceIndent - (cur.positionInBlock() % m_spaceIndent);

            QString text;
            text.fill(' ', indentRemain);
            cur.insertText(text);
        }
    }
*/
}

void CodeEditor::setTheme(const KSyntaxHighlighting::Theme &theme) {
    auto pal = qApp->palette();
    if(theme.isValid()) {
        pal.setColor(QPalette::Base, theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
        pal.setColor(QPalette::Text, theme.textColor(KSyntaxHighlighting::Theme::Normal));
        pal.setColor(QPalette::Highlight, theme.editorColor(KSyntaxHighlighting::Theme::TextSelection));
    }
    setPalette(pal);

    m_highlighter->setTheme(theme);
    m_highlighter->rehighlight();
    highlightCurrentLine();
}

int CodeEditor::sidebarWidth() const {
    int digits = 1;
    auto count = blockCount();
    while(count >= 10) {
        ++digits;
        count /= 10;
    }

    return 4 + fontMetrics().width(QLatin1Char('9')) * digits + 2 * fontMetrics().lineSpacing();
}

void CodeEditor::sidebarPaintEvent(QPaintEvent *event) {
    QPainter painter(m_sideBar);
    painter.fillRect(event->rect(), m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::IconBorder));

    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    const int currentBlockNumber = textCursor().blockNumber();

    const auto foldingMarkerSize = fontMetrics().lineSpacing();

    QFont f = font();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (m_displayLineNumbers && block.isVisible() && bottom >= event->rect().top()) {
            const auto number = QString::number(blockNumber + 1);
            bool current = (blockNumber == currentBlockNumber);
            painter.setPen(m_highlighter->theme().editorColor(
                current ? KSyntaxHighlighting::Theme::CurrentLineNumber
                        : KSyntaxHighlighting::Theme::LineNumbers));

            f.setBold(current);
            painter.setFont(f);
            painter.drawText(0, top, m_sideBar->width() - 2 - foldingMarkerSize, fontMetrics().height(), Qt::AlignRight, number);
        }

        // folding marker
        if (m_displayFoldingMarkers && block.isVisible() && isFoldable(block)) {
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
            painter.setBrush(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding)));
            painter.translate(m_sideBar->width() - foldingMarkerSize, top);
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
    m_sideBar->setGeometry(QRect(r.left(), r.top(), sidebarWidth(), r.height()));
    m_sideBar->repaint();
}

void CodeEditor::updateSidebarArea(const QRect& rect, int dy) {
    if(dy) {
        m_sideBar->scroll(0, dy);
    } else {
        m_sideBar->update(0, rect.y(), m_sideBar->width(), rect.height());
    }
}

void CodeEditor::highlightCurrentLine() {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CurrentLine)));
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
    return m_highlighter->startsFoldingRegion(block);
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
    if(m_displayFoldingMarkers) {
        auto endBlock = m_highlighter->findFoldingRegionEnd(startBlock).next();

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
            result -= (result % m_spaceIndent) + m_spaceIndent;
        } else {
            result++;
        }
    }
    return result;
}

int32_t CodeEditor::columnPosition(const QString &text, int column, int *offset) const {
    int32_t current = 0;
    int32_t result = 0;
    int32_t textSize = text.size();
    while((result < textSize) && current < column) {
        if(result < textSize && text.at(result) == QLatin1Char('\t')) {
            current -= (current % m_spaceIndent) + m_spaceIndent;
        } else {
            ++current;
        }
        ++result;
    }

    if(offset) {
        *offset = column - current;
    }
    return result;
}

QString CodeEditor::indentationString(int startColumn, int targetColumn, int padding, const QTextBlock &block) const {
    targetColumn = qMax(startColumn, targetColumn);
    return QString(targetColumn - startColumn, QLatin1Char(' '));
}

int CodeEditor::lineIndentPosition(const QString &text) const {
    int i = 0;
    while(i < text.size()) {
        if(!text.at(i).isSpace()) {
            break;
        }
        ++i;
    }
    int col = column(text, i);
    return i - (col % m_spaceIndent);
}

int CodeEditor::spacesLeftFromPosition(const QString &text, int position) const {
    if(position > text.size()) {
        return 0;
    }
    int i = position;
    while(i > 0) {
        if(!text.at(i-1).isSpace()) {
            break;
        }
        --i;
    }
    return position - i;
}

int CodeEditor::indentedColumn(int column, bool doIndent) const {
    int aligned = (column / m_spaceIndent) * m_spaceIndent;
    if (doIndent)
        return aligned + m_spaceIndent;
    if (aligned < column)
        return aligned;
    return qMax(0, aligned - m_spaceIndent);
}

QTextCursor CodeEditor::cursor() const {
    int32_t selectionAnchorColumn;
    int32_t selectionPositionColumn;

    int32_t firstBlock = qMin(m_blockPosition, m_blockAnchor);

    int32_t firstColumn = qMin(m_columnPosition, m_columnAnchor);
    int32_t lastColumn = qMax(m_columnPosition, m_columnAnchor);

    if(m_blockAnchor == m_blockPosition || true) {
        selectionAnchorColumn = m_columnAnchor;
        selectionPositionColumn = m_columnPosition;
    } else if(m_blockAnchor == firstBlock){
        selectionAnchorColumn = firstColumn;
        selectionPositionColumn = lastColumn;
    } else {
        selectionAnchorColumn = lastColumn;
        selectionPositionColumn = firstColumn;
    }

    QTextDocument *doc = document();
    QTextCursor cursor(doc);

    QTextBlock anchorTextBlock = doc->findBlockByNumber(m_blockAnchor);
    int32_t anchorPosition = anchorTextBlock.position() + columnPosition(anchorTextBlock.text(), selectionAnchorColumn);

    QTextBlock positionTextBlock = doc->findBlockByNumber(m_blockPosition);
    int32_t cursorPosition = positionTextBlock.position() + columnPosition(positionTextBlock.text(), selectionPositionColumn);

    cursor.setPosition(anchorPosition);
    cursor.setPosition(cursorPosition, QTextCursor::KeepAnchor);

    return cursor;
}

void CodeEditor::enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn) {
    m_blockPosition = positionBlock;
    m_columnPosition = positionColumn;

    m_blockAnchor = anchorBlock;
    m_columnAnchor = anchorColumn;

    m_blockSelection = true;
    doSetTextCursor(cursor(), true);
    viewport()->update();
}

void CodeEditor::disableBlockSelection() {
    m_blockSelection = false;

    m_blockPosition = 0;
    m_columnPosition = 0;

    m_blockAnchor = 0;
    m_columnAnchor = 0;

    viewport()->update();
}

void CodeEditor::doSetTextCursor(const QTextCursor &cursor, bool keepBlockSelection) {
    bool selectionChange = cursor.hasSelection() || textCursor().hasSelection();
    if(!keepBlockSelection && m_blockSelection) {
        m_blockSelection = false;
    }
    QTextCursor c = cursor;
    c.setVisualNavigation(true);
    QPlainTextEdit::doSetTextCursor(c);
}

void CodeEditor::doSetTextCursor(const QTextCursor &cursor) {
    doSetTextCursor(cursor, false);
}

void CodeEditor::setupSelections(const QTextBlock &block, int position, int length, QVector<QTextLayout::FormatRange> &selections) const {
    auto context = getPaintContext();

    int blockSelectionIndex = context.selections.size() - 1;

    for(int i = 0; i < context.selections.size(); ++i) {
        const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
        const int selStart = range.cursor.selectionStart() - position;
        const int selEnd = range.cursor.selectionEnd() - position;
        if(selStart < length && selEnd > 0 && selEnd > selStart) {
            QTextLayout::FormatRange o;
            o.start = selStart;
            o.length = selEnd - selStart;
            o.format = range.format;
            if(m_blockSelection && i == blockSelectionIndex) {
                QString text = block.text();

                o.start = columnPosition(text, qMin(m_columnPosition, m_columnAnchor));
                o.length = columnPosition(text, qMax(m_columnPosition, m_columnAnchor)) - o.start;
            }
            selections.append(o);
        } else if(!range.cursor.hasSelection() && range.format.hasProperty(QTextFormat::FullWidthSelection)
                   && block.contains(range.cursor.position()) && !m_blockSelection) {
            QTextLayout::FormatRange o;
            QTextLine l = block.layout()->lineForTextPosition(range.cursor.position() - position);
            o.start = l.textStart();
            o.length = l.textLength();
            if(o.start + o.length == length - 1) {
                ++o.length; // include newline
            }
            o.format = range.format;
            selections.append(o);
        }
    }
}

void CodeEditor::paintBlockSelection(const QTextBlock &block, QPainter &painter, const QPointF &offset, QRectF &blockRect) const {
    if (!m_blockSelection
            || block.blockNumber() < qMin(m_blockPosition, m_blockAnchor)
            || block.blockNumber() > qMax(m_blockPosition, m_blockAnchor)) {
        return;
    }

    QTextLayout *layout = block.layout();
    QRectF blockBounding = blockBoundingRect(block).translated(offset);
    QString text = block.text();

    const qreal spacew = QFontMetricsF(font()).width(QLatin1Char(' '));
    const int cursorw = overwriteMode() ? QFontMetrics(font()).width(QLatin1Char(' ')) : cursorWidth();

    int startOffset = 0;
    int relativePos = columnPosition(text, qMin(m_columnPosition, m_columnAnchor), &startOffset);
    const QTextLine line = layout->lineForTextPosition(relativePos);
    const qreal startX = line.cursorToX(relativePos) + startOffset * spacew;

    int endOffset = 0;
    int endRelativePos = columnPosition(text, qMax(m_columnPosition, m_columnAnchor), &endOffset);
    const QTextLine eline = layout->lineForTextPosition(endRelativePos);
    const qreal endX = eline.cursorToX(endRelativePos) + endOffset * spacew;

    QRectF rect = line.naturalTextRect();
    rect.moveTop(rect.top() + blockBounding.top());
    rect.setLeft(blockBounding.left() + startX);
    if(line.lineNumber() == eline.lineNumber()) {
        rect.setRight(blockBounding.left() + endX);
    }
    painter.fillRect(rect, palette().highlight());

    blockRect = rect;

    for(int i = line.lineNumber() + 1; i < eline.lineNumber(); ++i) {
        rect = layout->lineAt(i).naturalTextRect();
        rect.moveTop(rect.top() + blockBounding.top());
        painter.fillRect(rect, palette().highlight());
    }

    rect = eline.naturalTextRect();
    rect.moveTop(rect.top() + blockBounding.top());
    rect.setRight(blockBounding.left() + endX);
    if(line.lineNumber() != eline.lineNumber()) {
        painter.fillRect(rect, palette().highlight());
    }

    if(m_columnAnchor < m_columnPosition) {
        blockRect.setLeft(rect.right());
    }
    blockRect.setWidth(cursorw);
}

QString CodeEditor::copyBlockSelection() {
    if(!m_blockSelection) {
        return QString();
    }
    QString selection;
    QTextBlock block = document()->findBlockByNumber(qMin(m_blockPosition, m_blockAnchor));
    const QTextBlock &lastBlock = document()->findBlockByNumber(qMax(m_blockPosition, m_blockAnchor));
    bool textInserted = false;
    while(true) {
        if(textInserted) {
            selection += QLatin1Char('\n');
        }
        textInserted = true;

        QString text = block.text();
        int startOffset = 0;
        int startPos = columnPosition(text, qMin(m_columnPosition, m_columnAnchor), &startOffset);
        int endOffset = 0;
        int endPos = columnPosition(text, qMax(m_columnPosition, m_columnAnchor), &endOffset);

        if(startPos == endPos) {
            selection += QString(endOffset - startOffset, QLatin1Char(' '));
        } else {
            if(startOffset < 0) {
                selection += QString(-startOffset, QLatin1Char(' '));
            }
            if(endOffset < 0) {
                --endPos;
            }
            selection += text.midRef(startPos, endPos - startPos);
            if(endOffset < 0) {
                selection += QString((m_spaceTabs) ? m_spaceIndent : 1 + endOffset, QLatin1Char(' '));
            } else if (endOffset > 0) {
                selection += QString(endOffset, QLatin1Char(' '));
            }
        }

        if(block == lastBlock) {
            break;
        }

        block = block.next();
    }
    return selection;
}

void CodeEditor::removeBlockSelection() {
    QTextCursor cur = textCursor();

    const int firstColumn = qMin(m_columnPosition, m_columnAnchor);
    const int lastColumn = qMax(m_columnPosition, m_columnAnchor);
    if(firstColumn == lastColumn) {
        return;
    }
    const int positionBlock = m_blockPosition;
    const int anchorBlock = m_blockAnchor;

    int cursorPosition = cur.selectionStart();
    cur.clearSelection();
    cur.beginEditBlock();

    QTextBlock block = document()->findBlockByNumber(qMin(m_blockPosition, m_blockAnchor));
    const QTextBlock &lastBlock = document()->findBlockByNumber(qMax(m_blockPosition, m_blockAnchor));
    while(true) {
        int startOffset = 0;
        const int startPos = columnPosition(block.text(), firstColumn, &startOffset);
        // removing stuff doesn't make sense if the cursor is behind the code
        if(startPos < block.length() - 1 || startOffset < 0) {
            cur.setPosition(block.position());
            setCursorToColumn(cur, firstColumn);
            setCursorToColumn(cur, lastColumn, QTextCursor::KeepAnchor);
            cur.removeSelectedText();
        }
        if(block == lastBlock) {
            break;
        }
        block = block.next();
    }

    cur.setPosition(cursorPosition);
    cur.endEditBlock();

    m_blockPosition = positionBlock;
    m_columnPosition = firstColumn;

    m_blockAnchor = anchorBlock;
    m_columnAnchor = firstColumn;

    bool hasSelection = !(m_blockPosition == m_blockAnchor && m_columnPosition == m_columnAnchor);
    doSetTextCursor(cursor(), hasSelection);
}

void CodeEditor::insertIntoBlockSelection(const QString &text) {
    QTextCursor cur = textCursor();
    cur.beginEditBlock();

    if(overwriteMode() && qMax(m_columnPosition, m_columnAnchor) == m_columnPosition) {
        ++m_columnPosition;
    }

    if(m_columnPosition != m_columnAnchor) {
        removeBlockSelection();
        if(!m_blockSelection) {
            insertPlainText(text);
            cur.endEditBlock();
            return;
        }
    }

    if(text.isEmpty()) {
        cur.endEditBlock();
        return;
    }

    int positionBlock = m_blockPosition;
    int anchorBlock = m_blockAnchor;
    int column = m_columnPosition;

    const QTextBlock &firstBlock = document()->findBlockByNumber(qMin(m_blockPosition, m_blockAnchor));
    QTextBlock block = document()->findBlockByNumber(qMax(m_blockPosition, m_blockAnchor));

    const int selectionLineCount = qMax(m_blockPosition, m_blockAnchor) - qMin(m_blockPosition, m_blockAnchor);
    const int textNewLineCount = text.count(QLatin1Char('\n'));
    QStringList textLines = text.split(QLatin1Char('\n'));

    int textLength = 0;
    const QStringList::const_iterator endLine = textLines.constEnd();
    for(QStringList::const_iterator textLine = textLines.constBegin(); textLine != endLine; ++textLine) {
        textLength += qMax(0, columnPosition(*textLine, column) - textLength);
    }
    for(QStringList::iterator textLine = textLines.begin(); textLine != textLines.end(); ++textLine) {
        textLine->append(QString(qMax(0, textLength - columnPosition(*textLine, column)), QLatin1Char(' ')));
    }

    while(true) {
        cur.setPosition(block.position());
        if(selectionLineCount == textNewLineCount) {
            setCursorToColumn(cur, column);
            cur.insertText(textLines.at(block.blockNumber() - qMin(m_blockPosition, m_blockAnchor)));
        } else {
            QStringList::const_iterator textLine = textLines.constBegin();
            while(true) {
                setCursorToColumn(cur, column);
                cur.insertText(*textLine);
                ++textLine;
                if(textLine == endLine) {
                    break;
                }
                cur.movePosition(QTextCursor::EndOfBlock);
                cur.insertText(QLatin1String("\n"));
                if(qMax(anchorBlock, positionBlock) == anchorBlock) {
                    ++anchorBlock;
                } else {
                    ++positionBlock;
                }
            }
        }
        if(block == firstBlock) {
            break;
        }
        block = block.previous();
    }
    cur.endEditBlock();

    column += textLength;

    m_blockPosition = positionBlock;
    m_columnPosition = column;

    m_blockAnchor = anchorBlock;
    m_columnAnchor = column;

    doSetTextCursor(cursor(), true);
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

void CodeEditor::setCursorToColumn(QTextCursor &cursor, int column, QTextCursor::MoveMode moveMode) {
    int offset = 0;
    const int cursorPosition = cursor.position();
    const int pos = columnPosition(cursor.block().text(), column, &offset);
    cursor.setPosition(cursor.block().position() + pos, offset == 0 ? moveMode : QTextCursor::MoveAnchor);
    if(offset == 0) {
        return;
    }
    if(offset < 0) {
        cursor.setPosition(cursor.block().position() + pos - 1, QTextCursor::KeepAnchor);
        cursor.insertText(indentationString(
                              CodeEditor::column(cursor.block().text(), pos - 1),
                              CodeEditor::column(cursor.block().text(), pos), 0, cursor.block()));
    } else {
        cursor.insertText(indentationString(CodeEditor::column(cursor.block().text(), pos), column, 0, cursor.block()));
    }
    if(moveMode == QTextCursor::KeepAnchor) {
        cursor.setPosition(cursorPosition);
    }
    cursor.setPosition(cursor.block().position() + columnPosition(cursor.block().text(), column), moveMode);
}

void CodeEditor::insertFromMimeData(const QMimeData *source) {
    if(!isReadOnly() && m_blockSelection) {
        insertIntoBlockSelection(source->text());
        return;
    }
    QPlainTextEdit::insertFromMimeData(source);
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

void CodeEditor::onClassModelChanged() {
    if(m_classModel) {
        QStringList classes;
        QStringList enums;
        for(int row = 0; row < m_classModel->rowCount(); row++) {
            auto index = m_classModel->index(row, 0);
            switch(m_classModel->data(index, Qt::UserRole).toInt()) {
                case 4: enums << m_classModel->data(index).toString(); break;
                default: classes << m_classModel->data(index).toString(); break;
            }
        }
        m_definition.setKeywordList("classes", classes);
        m_definition.setKeywordList("enums", enums);
        m_highlighter->rehighlight();
    }
}
