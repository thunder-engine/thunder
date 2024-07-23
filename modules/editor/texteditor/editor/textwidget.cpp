#include "textwidget.h"

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

#include <editor/editorsettings.h>
#include <editor/pluginmanager.h>
#include <editor/codebuilder.h>

#include "codehandler.h"

namespace {
    const char *gFont("Editors/Text_Editor/Font/Font_Name");
    const char *gZoom("Editors/Text_Editor/Font/Zoom");

    const char *gLineNumbers("Editors/Text_Editor/Display/Line_Numbers");
    const char *gFoldingMarkers("Editors/Text_Editor/Display/Folding_Markers");
    const char *gWhitespaces("Editors/Text_Editor/Display/Whitespaces");

    const char *gSpaces("Editors/Text_Editor/Indents/Use_Spaces");
    const char *gTabSize("Editors/Text_Editor/Indents/Tab_Size");

    const char *gDefaultFont("Source Code Pro");
};

TextWidget::TextWidget(QWidget *parent) :
        QPlainTextEdit(parent),
        m_highlighter(new KSyntaxHighlighting::SyntaxHighlighter(document())),
        m_handler(new CodeHandler(this)),
        m_classModel(nullptr),
        m_sideBar(new TextWidgetSidebar(this)),
        m_blockSelection(false),
        m_blockPosition(0),
        m_columnPosition(0),
        m_blockAnchor(0),
        m_columnAnchor(0),
        m_spaceIndent(4),
        m_spaceTabs(true),
        m_displayLineNumbers(true),
        m_displayFoldingMarkers(true),
        m_cursorVisible(false) {

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_repository.addCustomSearchPath(":/Themes");
    setTheme(m_repository.theme("Thunder Dark"));

    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextWidget::updateSidebarGeometry);
    connect(this, &QPlainTextEdit::updateRequest, this, &TextWidget::updateSidebarArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextWidget::highlightCurrentLine);

    QTextOption option = document()->defaultTextOption();
    option.setFlags(option.flags() | QTextOption::AddSpaceForLineAndParagraphSeparators);
    document()->setDefaultTextOption(option);

    EditorSettings *settings = EditorSettings::instance();
    settings->value(gFont, gDefaultFont);
    settings->value(gZoom, QVariant::fromValue(100));

    settings->value(gLineNumbers, QVariant::fromValue(true));
    settings->value(gFoldingMarkers, QVariant::fromValue(true));
    settings->value(gWhitespaces, QVariant::fromValue(false));

    settings->value(gSpaces, QVariant::fromValue(true));
    settings->value(gTabSize, QVariant::fromValue(4));

    connect(settings, &EditorSettings::updated, this, &TextWidget::onApplySettings);

    onApplySettings();

    startTimer(500);
}

TextWidget::~TextWidget() {
    delete m_highlighter;
    delete m_sideBar;
}

void TextWidget::openFile(const QString &fileName) {
    m_fileName = fileName;
    QFile fp(m_fileName);
    if(!fp.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << m_fileName << ":" << fp.errorString();
        return;
    }
    clear();

    loadDefinition(m_fileName);

    checkClassMap();

    if(m_classModel) {
        connect(m_classModel, &QAbstractItemModel::layoutChanged, this, &TextWidget::onClassModelChanged);
        onClassModelChanged();
    }

    setPlainText(QString::fromUtf8(fp.readAll()));
}

void TextWidget::saveFile(const QString &path) {
    if(!path.isEmpty()) {
        m_fileName = path;
    }
    QFile fp(m_fileName);
    if(!fp.open(QFile::WriteOnly)) {
        qWarning() << "Failed to open" << m_fileName << ":" << fp.errorString();
        return;
    }

    fp.write(toPlainText().toUtf8());
    fp.close();

    document()->setModified(false);
}

void TextWidget::checkClassMap() {
    for(auto &it : PluginManager::instance()->extensions("converter")) {
        AssetConverter *converter = reinterpret_cast<AssetConverter *>(PluginManager::instance()->getPluginObject(it));

        for(QString &format : converter->suffixes()) {
            if(format.toLower() == QFileInfo(m_fileName).suffix()) {
                CodeBuilder *builder = dynamic_cast<CodeBuilder *>(converter);
                if(builder) {
                    m_classModel = builder->classMap();
                    return;
                }
            }
        }
    }
}

void TextWidget::loadDefinition(const QString &name) {
    m_definition = m_repository.definitionForFileName(name);
    m_highlighter->setDefinition(m_definition);
}

void TextWidget::setSpaceTabs(bool enable, uint32_t indent) {
    m_spaceTabs = enable;
    m_spaceIndent = (indent == 0) ? 4 : indent;
}

void TextWidget::displayLineNumbers(bool visible) {
    m_displayLineNumbers = visible;
    updateSidebarGeometry();
}

void TextWidget::displayFoldingMarkers(bool visible) {
    m_displayFoldingMarkers = visible;
    updateSidebarGeometry();
}

void TextWidget::decorateWhitespaces(bool value) {
    QTextOption option = document()->defaultTextOption();
    if(value) {
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    } else {
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
    }
    document()->setDefaultTextOption(option);
}

void TextWidget::highlightBlock(const QString &text) {
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

bool TextWidget::findString(const QString &string, bool reverse, bool casesens, bool words)  {
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

void TextWidget::replaceSelected(const QString &string) {
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

void TextWidget::reportIssue(int level, int line, int col, const QString &text) {

}

void TextWidget::mousePressEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        if(event->modifiers() & Qt::AltModifier) {
            QTextCursor cur = cursorForPosition(event->pos());
            int col = column(cur.block().text(), cur.positionInBlock());
            if (cur.positionInBlock() == cur.block().length() - 1) {
                col += (event->pos().x() - cursorRect(cur).center().x()) / QFontMetricsF(font()).boundingRect(' ').width();
            }

            int block = cur.blockNumber();
            if(block == blockCount() - 1) {
                block += (event->pos().y() - cursorRect(cur).center().y()) / QFontMetricsF(font()).lineSpacing();
            }
            if(blockSelection()) {
                setBlockPosition(block);
                setColumnPosition(col);

                doSetTextCursor(cursor(), true);
                viewport()->update();
            } else {
                enableBlockSelection(block, col, block, col);
            }
        } else {
            if(blockSelection()) {
                disableBlockSelection();
            }
        }
    }

    QPlainTextEdit::mousePressEvent(event);
}

void TextWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        if(event->modifiers() & Qt::AltModifier) {
            if(blockSelection()) {
                QTextCursor cur = textCursor();
                int32_t col = column(cur.block().text(), cur.positionInBlock());
                if(cur.positionInBlock() == cur.block().length() - 1) {
                    col += (event->pos().x() - cursorRect().center().x()) / QFontMetricsF(font()).boundingRect(' ').width();
                }

                setBlockPosition(cur.blockNumber());
                setColumnPosition(col);

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
                        col += (event->pos().x() - cursorRect().center().x()) / QFontMetricsF(font()).boundingRect(' ').width();
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

    QPlainTextEdit::mouseMoveEvent(event);
}

void TextWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = createStandardContextMenu(event->pos());
    menu->exec(event->globalPos());
    delete menu;
}

void TextWidget::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    updateSidebarGeometry();
}

void TextWidget::paintEvent(QPaintEvent *event) {
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

            bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard)) &&
                               context.cursorPosition >= blpos &&
                               context.cursorPosition < blpos + bllen);
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
                if((drawCursor && !drawCursorAsBlock) ||
                        (editable && context.cursorPosition < -1 && !layout->preeditAreaText().isEmpty())) {
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

void TextWidget::timerEvent(QTimerEvent *event) {
    m_cursorVisible = !m_cursorVisible;
    update();
}

void TextWidget::setTheme(const KSyntaxHighlighting::Theme &theme) {
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

int TextWidget::sidebarWidth() const {
    int digits = 1;
    auto count = blockCount();
    while(count >= 10) {
        ++digits;
        count /= 10;
    }

    return 4 + fontMetrics().boundingRect('9').width() * digits + 2 * fontMetrics().lineSpacing();
}

void TextWidget::sidebarPaintEvent(QPaintEvent *event) {
    QPainter painter(m_sideBar);
    painter.fillRect(event->rect(), m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::IconBorder));

    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    const int currentBlockNumber = textCursor().blockNumber();

    const auto foldingMarkerSize = fontMetrics().lineSpacing();

    QFont f = font();
    while(block.isValid() && top <= event->rect().bottom()) {
        if(m_displayLineNumbers && block.isVisible() && bottom >= event->rect().top()) {
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
        if(m_displayFoldingMarkers && block.isVisible() && isFoldable(block)) {
            QPolygonF polygon;
            if(isFolded(block)) {
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

void TextWidget::updateSidebarGeometry() {
    setViewportMargins(sidebarWidth(), 0, 0, 0);
    const auto r = contentsRect();
    m_sideBar->setGeometry(QRect(r.left(), r.top(), sidebarWidth(), r.height()));
    m_sideBar->repaint();
}

void TextWidget::updateSidebarArea(const QRect& rect, int dy) {
    if(dy) {
        m_sideBar->scroll(0, dy);
    } else {
        m_sideBar->update(0, rect.y(), m_sideBar->width(), rect.height());
    }
}

void TextWidget::highlightCurrentLine() {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CurrentLine)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

bool TextWidget::useSpaces() const {
    return m_spaceTabs;
}

int32_t TextWidget::spaceIndent() const {
    return m_spaceIndent;
}

QTextBlock TextWidget::blockAtPosition(int y) const {
    auto block = firstVisibleBlock();
    if(!block.isValid()) {
        return QTextBlock();
    }

    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    do {
        if(top <= y && y <= bottom) {
            return block;
        }
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    } while(block.isValid());
    return QTextBlock();
}

bool TextWidget::isFoldable(const QTextBlock &block) const {
    return m_highlighter->startsFoldingRegion(block);
}

bool TextWidget::isFolded(const QTextBlock &block) const {
    if(!block.isValid()) {
        return false;
    }
    const auto nextBlock = block.next();
    if(!nextBlock.isValid()) {
        return false;
    }
    return !nextBlock.isVisible();
}

void TextWidget::toggleFold(const QTextBlock &startBlock) {
    if(m_displayFoldingMarkers) {
        auto endBlock = m_highlighter->findFoldingRegionEnd(startBlock).next();

        endBlock = endBlock.previous();

        if(isFolded(startBlock)) { // unfold
            auto block = startBlock.next();
            while(block.isValid() && !block.isVisible()) {
                block.setVisible(true);
                block.setLineCount(block.layout()->lineCount());
                block = block.next();
            }
        } else { // fold
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

void TextWidget::doSetTextCursor(const QTextCursor &cursor, bool keepBlockSelection) {
    if(!keepBlockSelection && m_blockSelection) {
        m_blockSelection = false;
    }
    QTextCursor c = cursor;
    c.setVisualNavigation(true);
    QPlainTextEdit::doSetTextCursor(c);
}

void TextWidget::doSetTextCursor(const QTextCursor &cursor) {
    doSetTextCursor(cursor, false);
}

void TextWidget::setupSelections(const QTextBlock &block, int position, int length, QVector<QTextLayout::FormatRange> &selections) const {
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

void TextWidget::paintBlockSelection(const QTextBlock &block, QPainter &painter, const QPointF &offset, QRectF &blockRect) const {
    if (!m_blockSelection ||
            block.blockNumber() < qMin(m_blockPosition, m_blockAnchor) ||
            block.blockNumber() > qMax(m_blockPosition, m_blockAnchor)) {
        return;
    }

    QTextLayout *layout = block.layout();
    QRectF blockBounding = blockBoundingRect(block).translated(offset);
    QString text = block.text();

    const qreal spacew = QFontMetricsF(font()).boundingRect(' ').width();
    const int cursorw = overwriteMode() ? spacew : cursorWidth();

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

QTextCursor TextWidget::cursor() const {
    int32_t selectionAnchorColumn;
    int32_t selectionPositionColumn;

    if(m_blockAnchor == m_blockPosition) {
        selectionAnchorColumn = m_columnAnchor;
        selectionPositionColumn = m_columnPosition;
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

void TextWidget::enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn) {
    m_blockPosition = positionBlock;
    m_columnPosition = positionColumn;

    m_blockAnchor = anchorBlock;
    m_columnAnchor = anchorColumn;

    m_blockSelection = true;
    doSetTextCursor(cursor(), true);

    viewport()->update();
}

void TextWidget::disableBlockSelection() {
    m_blockSelection = false;

    m_blockPosition = 0;
    m_columnPosition = 0;

    m_blockAnchor = 0;
    m_columnAnchor = 0;

    viewport()->update();
}

bool TextWidget::blockSelection() const {
    return m_blockSelection;
}

void TextWidget::setBlockSelection(bool selection) {
    m_blockSelection = selection;
}

int32_t TextWidget::blockPosition() const {
    return m_blockPosition;
}

void TextWidget::setBlockPosition(int32_t position) {
    m_blockPosition = position;
}

int32_t TextWidget::columnPosition() const {
    return m_columnPosition;
}

void TextWidget::setColumnPosition(int32_t position) {
    m_columnPosition = position;
}

int32_t TextWidget::blockAnchor() const {
    return m_blockAnchor;
}
void TextWidget::setBlockAnchor(int32_t anchor) {
    m_blockAnchor = anchor;
}

int32_t TextWidget::columnAnchor() const {
    return m_columnAnchor;
}
void TextWidget::setColumnAnchor(int32_t anchor) {
    m_columnAnchor = anchor;
}

int32_t TextWidget::column(const QString &text, int32_t pos) const {
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

int32_t TextWidget::columnPosition(const QString &text, int column, int *offset) const {
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

void TextWidget::insertFromMimeData(const QMimeData *source) {
    if(!isReadOnly() && m_blockSelection) {
        //insertIntoBlockSelection(source->text());
        return;
    }
    QPlainTextEdit::insertFromMimeData(source);
}

void TextWidget::onApplySettings() {
    EditorSettings *s = EditorSettings::instance();
    QString name = s->property(gFont).toString();
    if(name.isEmpty()) {
        name = gDefaultFont;
        s->setProperty(gFont, name);
    }
    QFont font(name, 10);
    font.setFixedPitch(true);
    setFont(font);

    int zoom = s->property(gZoom).toInt();
    if(zoom <= 0) {
        zoom = 100;
        s->setProperty(gZoom, zoom);
    }

    int32_t range = 0.01f * font.pointSize() * float(zoom) - font.pointSize();
    zoomIn(range);

    displayLineNumbers(s->property(gLineNumbers).toBool());
    displayFoldingMarkers(s->property(gFoldingMarkers).toBool());

    decorateWhitespaces(s->property(gWhitespaces).toBool());

    setSpaceTabs(s->property(gSpaces).toBool(), s->property(gTabSize).toInt());
}

void TextWidget::onClassModelChanged() {
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
