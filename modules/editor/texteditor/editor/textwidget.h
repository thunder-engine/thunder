#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <QPlainTextEdit>
#include <QTextBlock>

#include <repository.h>
#include <definition.h>

namespace KSyntaxHighlighting {
    class SyntaxHighlighter;
    class Theme;
}

class TextWidgetSidebar;
class QAbstractItemModel;
class CodeHandler;

class TextWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit TextWidget(QWidget *parent = nullptr);
    ~TextWidget() override;

    void openFile(const QString &fileName);
    void saveFile(const QString &path = QString());

    void loadDefinition(const QString &name);

    bool useSpaces() const;
    int32_t spaceIndent() const;

    QTextBlock blockAtPosition(int y) const;
    bool isFoldable(const QTextBlock &block) const;
    bool isFolded(const QTextBlock &block) const;
    void toggleFold(const QTextBlock &block);

    void setSpaceTabs(bool enable, uint32_t indent);
    void displayLineNumbers(bool visible);
    void displayFoldingMarkers(bool visible);

    void decorateWhitespaces(bool value);

    void highlightBlock(const QString &text);
    bool findString(const QString &string, bool reverse, bool casesens = true, bool words = false);
    void replaceSelected(const QString &string);

    void reportIssue(int level, int line, int col, const QString &text);

    void doSetTextCursor(const QTextCursor &cursor, bool keepBlockSelection);
    void doSetTextCursor(const QTextCursor &cursor) Q_DECL_OVERRIDE;

    int32_t column(const QString &text, int32_t pos) const;
    int32_t columnPosition(const QString &text, int column, int *offset = nullptr) const;

    QTextCursor cursor() const;
    void enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn);
    void disableBlockSelection();

    bool blockSelection() const;
    void setBlockSelection(bool selection);

    int32_t blockPosition() const;
    void setBlockPosition(int32_t position);

    int32_t columnPosition() const;
    void setColumnPosition(int32_t position);

    int32_t blockAnchor() const;
    void setBlockAnchor(int32_t anchor);

    int32_t columnAnchor() const;
    void setColumnAnchor(int32_t anchor);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onApplySettings();
    void onClassModelChanged();

private:
    friend class TextWidgetSidebar;

    void checkClassMap();

    void setTheme(const KSyntaxHighlighting::Theme &theme);
    int sidebarWidth() const;
    void sidebarPaintEvent(QPaintEvent *event);
    void updateSidebarGeometry();
    void updateSidebarArea(const QRect &rect, int dy);
    void highlightCurrentLine();

    void setupSelections(const QTextBlock &block, int position, int length, QVector<QTextLayout::FormatRange> &selections) const;
    void paintBlockSelection(const QTextBlock &block, QPainter &painter, const QPointF &offset, QRectF &blockRect) const;

    void insertFromMimeData(const QMimeData *source) Q_DECL_OVERRIDE;

private:
    QString m_fileName;

    KSyntaxHighlighting::Definition m_definition;
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter;

    CodeHandler *m_handler;

    QAbstractItemModel *m_classModel;

    TextWidgetSidebar *m_sideBar;

    bool m_blockSelection;
    int32_t m_blockPosition;
    int32_t m_columnPosition;

    int32_t m_blockAnchor;
    int32_t m_columnAnchor;

    int32_t m_spaceIndent;
    bool m_spaceTabs;

    bool m_displayLineNumbers;
    bool m_displayFoldingMarkers;

    bool m_cursorVisible;
};

class TextWidgetSidebar : public QWidget {
    Q_OBJECT
public:
    explicit TextWidgetSidebar(TextWidget *editor) :
            QWidget(editor),
            m_textWidget(editor) {
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(m_textWidget->sidebarWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        m_textWidget->sidebarPaintEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE {
        if(event->x() >= width() - m_textWidget->fontMetrics().lineSpacing()) {
            auto block = m_textWidget->blockAtPosition(event->y());
            if(!block.isValid() || !m_textWidget->isFoldable(block)) {
                return;
            }
            m_textWidget->toggleFold(block);
        }

        QWidget::mouseReleaseEvent(event);
    }

private:
    TextWidget *m_textWidget;
};

#endif // TEXTWIDGET_H
