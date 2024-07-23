#include "codehandler.h"

#include "textwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QTextBlock>

CodeHandler::CodeHandler(TextWidget *widget) :
        QObject(widget),
        m_widget(widget) {

    m_widget->installEventFilter(this);
}

bool CodeHandler::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        return keyPress(static_cast<QKeyEvent *>(event));
    }

    return false;
}

bool CodeHandler::keyPress(QKeyEvent *event) {
    if(m_widget == nullptr || m_widget->isReadOnly()) {
        event->accept();
        return true;
    }

    QTextCursor cursor = m_widget->textCursor();

    if(event == QKeySequence::InsertParagraphSeparator) {
        cursor.beginEditBlock();

        QString text;

        QTextBlock block = cursor.block();
        while(block.isValid() && text.trimmed().isEmpty()) {
            text = block.text();
            block = block.previous();
        }

        int32_t indentRemain = firstNonIndent(text);
        if(m_widget->isFoldable(cursor.block())) {
            indentRemain += m_widget->useSpaces() ? m_widget->spaceIndent() : 1;
        }

        cursor.insertBlock();
        if(m_widget->useSpaces()) {
            text.fill(' ', indentRemain * m_widget->spaceIndent());
        } else {
            text.fill('\t', indentRemain);
        }
        cursor.insertText(text);

        cursor.endEditBlock();

        event->accept();
        return true;
    } else if(event == QKeySequence::MoveToStartOfLine || event == QKeySequence::SelectStartOfLine) {
        int32_t indentRemain = firstNonIndent(cursor.block().text()) * (m_widget->useSpaces() ? m_widget->spaceIndent() : 1);
        int32_t pos = cursor.block().position() + indentRemain;
        if(cursor.position() > pos) {
            cursor.setPosition(pos, event == QKeySequence::SelectStartOfLine ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

            m_widget->doSetTextCursor(cursor, true);

            event->accept();
            return true;
        }
    }

    if(m_widget->blockSelection()) {
        if(event == QKeySequence::Copy) {
            QApplication::clipboard()->setText(copyBlockSelection());

            event->accept();
            return true;
        } else if(event == QKeySequence::Cut) {
            QApplication::clipboard()->setText(copyBlockSelection());
            removeBlockSelection();

            event->accept();
            return true;
        } else if(event == QKeySequence::Delete || event->key() == Qt::Key_Backspace) {
            if(m_widget->columnPosition() == m_widget->columnAnchor()) {
                if(event == QKeySequence::Delete) {
                    m_widget->setColumnPosition(m_widget->columnPosition() + 1);
                } else if(m_widget->columnPosition() > 0) {
                    m_widget->setColumnPosition(m_widget->columnPosition() - 1);
                }
            }
            removeBlockSelection();

            event->accept();
            return true;
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
                return true;
            }
        } break;
        case Qt::Key_Tab: {
            if(m_widget->blockSelection() &&
                    qMin(m_widget->columnPosition(), m_widget->columnAnchor()) != qMax(m_widget->columnPosition(), m_widget->columnAnchor())) {
                removeBlockSelection();
            } else {
                indentSelection();
            }
            event->accept();
            return true;
        } break;
        case Qt::Key_Insert: {
            if(event->modifiers() == Qt::NoModifier) {
                m_widget->setOverwriteMode(!m_widget->overwriteMode());
                event->accept();
                return true;
            }
        } break;
    }

    if(m_widget->blockSelection()) {
        const QString text = event->text();
        if(!text.isEmpty() &&
                (text.at(0).isPrint() || text.at(0) == QLatin1Char('\t'))) {
            insertIntoBlockSelection(text);

            return true;
        }
    }

    return false;
}

void CodeHandler::commentSelection() {
    QTextCursor cursor = m_widget->textCursor();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    bool hasSelection = cursor.hasSelection();

    QTextDocument *doc = m_widget->document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    static const QString singleLine("//");
    static const QString multiLineBegin("/*");
    static const QString multiLineEnd("*/");

    static const bool hasMultiLineStyle = true;
    static const bool hasSingleLineStyle = true;

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
        if(startPos >= multiLineStartLength &&
                startText.indexOf(multiLineBegin, pos) == pos) {
            startPos -= multiLineStartLength;
            start -= multiLineStartLength;
        }

        bool hasSelStart = (startPos <= startText.length() - multiLineStartLength) &&
                (startText.indexOf(multiLineBegin, startPos) == startPos);

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

        bool doSingleLineStyleUncomment = true;
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
        cursor = m_widget->textCursor();
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
        m_widget->setTextCursor(cursor);
    }
}

void CodeHandler::indentSelection() {
    QTextCursor cursor = m_widget->textCursor();

    cursor.beginEditBlock();

    int pos = cursor.position();
    int col = m_widget->blockSelection() ? m_widget->columnPosition() : m_widget->column(cursor.block().text(), cursor.positionInBlock());

    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    QTextDocument *doc = m_widget->document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(qMax(end - 1, 0)).next();

    const bool cursorAtBlockStart = (cursor.position() == startBlock.position());
    const bool anchorAtBlockStart = (cursor.anchor() == startBlock.position());
    const bool oneLinePartial = (startBlock.next() == endBlock) &&
            (start > startBlock.position() || end < endBlock.position() - 1);

    if(startBlock == endBlock) {
        endBlock = endBlock.next();
    }
    if(cursor.hasSelection() && !m_widget->blockSelection() && !oneLinePartial) {
        for(QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            const QString text = block.text();
            int indentPosition = lineIndentPosition(text);

            int targetColumn = indentedColumn(m_widget->column(text, indentPosition), true);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(indentationString(0, targetColumn, 0, block));
            cursor.setPosition(block.position());
            cursor.setPosition(block.position() + indentPosition, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        if (cursorAtBlockStart) {
            cursor = m_widget->textCursor();
            cursor.setPosition(startBlock.position(), QTextCursor::KeepAnchor);
        } else if(anchorAtBlockStart) {
            cursor = m_widget->textCursor();
            cursor.setPosition(startBlock.position(), QTextCursor::MoveAnchor);
            cursor.setPosition(m_widget->textCursor().position(), QTextCursor::KeepAnchor);
        }
    } else if(cursor.hasSelection() && !m_widget->blockSelection() && oneLinePartial) {
        cursor.removeSelectedText();
    } else {
        for(QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text();

            int blockColumn = m_widget->column(text, text.size());
            if(blockColumn < col) {
                cursor.setPosition(block.position() + text.size());
                cursor.insertText(indentationString(blockColumn, col, 0, block));
                text = block.text();
            }

            int indentPosition = m_widget->columnPosition(text, col, 0);
            int spaces = spacesLeftFromPosition(text, indentPosition);
            int startColumn = m_widget->column(text, indentPosition - spaces);
            int targetColumn = indentedColumn(m_widget->column(text, indentPosition), true);
            cursor.setPosition(block.position() + indentPosition);
            cursor.setPosition(block.position() + indentPosition - spaces, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.insertText(indentationString(startColumn, targetColumn, 0, block));
        }

        if(m_widget->blockSelection()) {
            end = cursor.position();
            int offset = m_widget->column(cursor.block().text(), cursor.positionInBlock()) - col;

            m_widget->setColumnAnchor(m_widget->columnAnchor() + offset);
            m_widget->setColumnPosition(m_widget->columnPosition() + offset);

            cursor.setPosition(start);
            cursor.setPosition(end, QTextCursor::KeepAnchor);
        }
    }

    cursor.endEditBlock();

    m_widget->doSetTextCursor(cursor, true);
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

QString CodeHandler::indentationString(int startColumn, int targetColumn, int padding, const QTextBlock &block) const {
    targetColumn = qMax(startColumn, targetColumn);
    return QString(targetColumn - startColumn, QLatin1Char(' '));
}

int CodeHandler::lineIndentPosition(const QString &text) const {
    int i = 0;
    while(i < text.size()) {
        if(!text.at(i).isSpace()) {
            break;
        }
        ++i;
    }

    return i - (m_widget->column(text, i) % m_widget->spaceIndent());
}

int CodeHandler::spacesLeftFromPosition(const QString &text, int position) const {
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

int CodeHandler::indentedColumn(int column, bool doIndent) const {
    int aligned = (column / m_widget->spaceIndent()) * m_widget->spaceIndent();
    if(doIndent) {
        return aligned + m_widget->spaceIndent();
    }
    if(aligned < column) {
        return aligned;
    }

    return qMax(0, aligned - m_widget->spaceIndent());
}

QString CodeHandler::copyBlockSelection() {
    if(!m_widget->blockSelection()) {
        return QString();
    }

    QTextDocument *doc = m_widget->document();

    QString selection;
    QTextBlock block = doc->findBlockByNumber(qMin(m_widget->blockPosition(), m_widget->blockAnchor()));
    const QTextBlock &lastBlock = doc->findBlockByNumber(qMax(m_widget->blockPosition(), m_widget->blockAnchor()));
    bool textInserted = false;
    while(true) {
        if(textInserted) {
            selection += QLatin1Char('\n');
        }
        textInserted = true;

        QString text = block.text();
        int startOffset = 0;
        int startPos = m_widget->columnPosition(text, qMin(m_widget->columnPosition(), m_widget->columnAnchor()), &startOffset);
        int endOffset = 0;
        int endPos = m_widget->columnPosition(text, qMax(m_widget->columnPosition(), m_widget->columnAnchor()), &endOffset);

        if(startPos == endPos) {
            selection += QString(endOffset - startOffset, QLatin1Char(' '));
        } else {
            if(startOffset < 0) {
                selection += QString(-startOffset, QLatin1Char(' '));
            }
            if(endOffset < 0) {
                --endPos;
            }
            selection += text.mid(startPos, endPos - startPos);
            if(endOffset < 0) {
                selection += QString((m_widget->useSpaces()) ? m_widget->spaceIndent() : 1 + endOffset, QLatin1Char(' '));
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

void CodeHandler::removeBlockSelection() {
    QTextCursor cur = m_widget->textCursor();

    const int firstColumn = qMin(m_widget->columnPosition(), m_widget->columnAnchor());
    const int lastColumn = qMax(m_widget->columnPosition(), m_widget->columnAnchor());
    if(firstColumn == lastColumn) {
        return;
    }
    const int positionBlock = m_widget->blockPosition();
    const int anchorBlock = m_widget->blockAnchor();

    int cursorPosition = cur.selectionStart();
    cur.clearSelection();
    cur.beginEditBlock();

    QTextDocument *doc = m_widget->document();

    QTextBlock block = doc->findBlockByNumber(qMin(positionBlock, anchorBlock));
    const QTextBlock &lastBlock = doc->findBlockByNumber(qMax(positionBlock, anchorBlock));
    while(true) {
        int startOffset = 0;
        const int startPos = m_widget->columnPosition(block.text(), firstColumn, &startOffset);
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

    m_widget->setBlockPosition(positionBlock);
    m_widget->setColumnPosition(firstColumn);

    m_widget->setBlockAnchor(anchorBlock);
    m_widget->setColumnAnchor(firstColumn);

    bool hasSelection = !(positionBlock != anchorBlock);
    m_widget->doSetTextCursor(m_widget->cursor(), hasSelection);
}

void CodeHandler::insertIntoBlockSelection(const QString &text) {
    QTextCursor cur = m_widget->textCursor();
    cur.beginEditBlock();

    int column = m_widget->columnPosition();

    if(m_widget->overwriteMode() && qMax(column, m_widget->columnAnchor()) == column) {
        ++column;
    }

    if(m_widget->columnPosition() != m_widget->columnAnchor()) {
        removeBlockSelection();
        if(!m_widget->blockSelection()) {
            m_widget->insertPlainText(text);
            cur.endEditBlock();
            return;
        }
    }

    if(text.isEmpty()) {
        cur.endEditBlock();
        return;
    }

    int positionBlock = m_widget->blockPosition();
    int anchorBlock = m_widget->blockAnchor();

    QTextDocument *doc = m_widget->document();
    const QTextBlock &firstBlock = doc->findBlockByNumber(qMin(positionBlock, anchorBlock));
    QTextBlock block = doc->findBlockByNumber(qMax(positionBlock, anchorBlock));

    const int selectionLineCount = qMax(positionBlock, anchorBlock) - qMin(positionBlock, anchorBlock);
    const int textNewLineCount = text.count(QLatin1Char('\n'));
    QStringList textLines = text.split(QLatin1Char('\n'));

    int textLength = 0;
    const QStringList::const_iterator endLine = textLines.constEnd();
    for(QStringList::const_iterator textLine = textLines.constBegin(); textLine != endLine; ++textLine) {
        textLength += qMax(0, m_widget->columnPosition(*textLine, column) - textLength);
    }
    for(QStringList::iterator textLine = textLines.begin(); textLine != textLines.end(); ++textLine) {
        textLine->append(QString(qMax(0, textLength - m_widget->columnPosition(*textLine, column)), QLatin1Char(' ')));
    }

    while(true) {
        cur.setPosition(block.position());
        if(selectionLineCount == textNewLineCount) {
            setCursorToColumn(cur, column);
            cur.insertText(textLines.at(block.blockNumber() - qMin(m_widget->blockPosition(), m_widget->blockAnchor())));
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

    m_widget->setBlockPosition(positionBlock);
    m_widget->setColumnPosition(column);

    m_widget->setBlockAnchor(anchorBlock);
    m_widget->setColumnAnchor(column);

    m_widget->doSetTextCursor(m_widget->cursor(), true);
}

int32_t CodeHandler::firstNonIndent(const QString &text) const {
    int32_t i = 0;
    int32_t spaceIndent = std::max(m_widget->spaceIndent(), 1);
    bool isSpace = false;
    while(i < text.size()) {
        if(text.at(i) != ' ') {
            if(text.at(i) != '\t') {
                return i / (isSpace ? spaceIndent : 1);
            }
        } else {
            isSpace = true;
        }
        ++i;
    }

    return i;
}

void CodeHandler::setCursorToColumn(QTextCursor &cursor, int column, QTextCursor::MoveMode moveMode) {
    int offset = 0;
    const int cursorPosition = cursor.position();
    const int pos = m_widget->columnPosition(cursor.block().text(), column, &offset);
    cursor.setPosition(cursor.block().position() + pos, offset == 0 ? moveMode : QTextCursor::MoveAnchor);

    if(offset == 0) {
        return;
    }

    if(offset < 0) {
        cursor.setPosition(cursor.block().position() + pos - 1, QTextCursor::KeepAnchor);
        cursor.insertText(indentationString( m_widget->column(cursor.block().text(), pos - 1),
                                             m_widget->column(cursor.block().text(), pos), 0, cursor.block()));
    } else {
        cursor.insertText(indentationString(m_widget->column(cursor.block().text(), pos), column, 0, cursor.block()));
    }

    if(moveMode == QTextCursor::KeepAnchor) {
        cursor.setPosition(cursorPosition);
    }
    cursor.setPosition(cursor.block().position() + m_widget->columnPosition(cursor.block().text(), column), moveMode);
}
