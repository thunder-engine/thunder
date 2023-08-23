#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QPlainTextEdit>
#include <QTextBlock>

#include <repository.h>
#include <definition.h>

namespace KSyntaxHighlighting {
    class SyntaxHighlighter;
    class Theme;
}

class CodeEditSidebar;
class QAbstractItemModel;

class CodeEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEdit(QWidget *parent = nullptr);
    ~CodeEdit() override;

    void openFile(const QString &fileName);
    void saveFile(const QString &path = QString());

    void loadDefinition(const QString &name);

    void setSpaceTabs(bool enable, uint32_t indent);
    void displayLineNumbers(bool visible);
    void displayFoldingMarkers(bool visible);

    void decorateWhitespaces(bool value);

    void highlightBlock(const QString &text);
    bool findString(const QString &string, bool reverse, bool casesens = true, bool words = false);
    void replaceSelected(const QString &string);

    void reportIssue(int level, int line, int col, const QString &text);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onApplySettings();
    void onClassModelChanged();

private:
    friend class CodeEditSidebar;

    void checkClassMap();

    void setTheme(const KSyntaxHighlighting::Theme &theme);
    int sidebarWidth() const;
    void sidebarPaintEvent(QPaintEvent *event);
    void updateSidebarGeometry();
    void updateSidebarArea(const QRect &rect, int dy);
    void highlightCurrentLine();

    QTextBlock blockAtPosition(int y) const;
    bool isFoldable(const QTextBlock &block) const;
    bool isFolded(const QTextBlock &block) const;
    void toggleFold(const QTextBlock &block);

    int32_t column(const QString &text, int32_t pos) const;
    int32_t columnPosition(const QString &text, int column, int *offset = nullptr) const;
    QString indentationString(int startColumn, int targetColumn, int padding, const QTextBlock &block) const;
    int lineIndentPosition(const QString &text) const;
    int spacesLeftFromPosition(const QString &text, int position) const;
    int indentedColumn(int column, bool doIndent) const;

    QTextCursor cursor() const;
    void enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn);
    void disableBlockSelection();

    void setupSelections(const QTextBlock &block, int position, int length, QVector<QTextLayout::FormatRange> &selections) const;
    void paintBlockSelection(const QTextBlock &block, QPainter &painter, const QPointF &offset, QRectF &blockRect) const;

    void commentSelection();
    void indentSelection();

    QString copyBlockSelection();
    void removeBlockSelection();
    void insertIntoBlockSelection(const QString &text);

    void doSetTextCursor(const QTextCursor &cursor, bool keepBlockSelection);
    void doSetTextCursor(const QTextCursor &cursor) Q_DECL_OVERRIDE;
    int32_t firstNonIndent(const QString &text) const;

    void setCursorToColumn(QTextCursor &cursor, int column, QTextCursor::MoveMode moveMode = QTextCursor::MoveAnchor);

    void insertFromMimeData(const QMimeData *source) Q_DECL_OVERRIDE;

    QString m_fileName;

    KSyntaxHighlighting::Definition m_definition;
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter;

    QAbstractItemModel *m_classModel;

    CodeEditSidebar *m_sideBar;

    bool m_spaceTabs;
    int32_t m_spaceIndent;

    bool m_blockSelection;
    int32_t m_blockPosition;
    int32_t m_columnPosition;

    int32_t m_blockAnchor;
    int32_t m_columnAnchor;

    bool m_displayLineNumbers;
    bool m_displayFoldingMarkers;

    bool m_cursorVisible;
};

class CodeEditSidebar : public QWidget {
    Q_OBJECT
public:
    explicit CodeEditSidebar(CodeEdit *editor) :
            QWidget(editor),
            m_codeEdit(editor) {
    }
    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(m_codeEdit->sidebarWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        m_codeEdit->sidebarPaintEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE {
        if(event->x() >= width() - m_codeEdit->fontMetrics().lineSpacing()) {
            auto block = m_codeEdit->blockAtPosition(event->y());
            if(!block.isValid() || !m_codeEdit->isFoldable(block)) {
                return;
            }
            m_codeEdit->toggleFold(block);
        }
        QWidget::mouseReleaseEvent(event);
    }

private:
    CodeEdit *m_codeEdit;
};

#endif // CODEEDIT_H
