#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QPlainTextEdit>
#include <QTextBlock>

#include <repository.h>

namespace KSyntaxHighlighting {
    class SyntaxHighlighter;
    class Theme;
}

class CodeEditorSidebar;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor() override;

    void openFile(const QString &fileName);
    void saveFile(const QString &path = QString());

    void setSpaceTabs(bool enable, uint32_t indent);
    void displayLineNumbers(bool visible);
    void displayFoldingMarkers(bool visible);

    void decorateWhitespaces(bool value);

    void highlightBlock(const QString &text);
    bool findString(const QString &string, bool reverse, bool casesens = true, bool words = false);
    void replaceSelected(const QString &string);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onApplySettings();

private:
    friend class CodeEditorSidebar;

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
    int32_t columnPosition(const QString &text, int column) const;
    QTextCursor cursor() const;
    void enableBlockSelection(int32_t positionBlock, int32_t positionColumn, int32_t anchorBlock, int32_t anchorColumn);
    void disableBlockSelection();

    void doSetTextCursor(const QTextCursor &cursor) Q_DECL_OVERRIDE;
    int32_t firstNonIndent(const QString &text) const;

    QString m_FileName;

    KSyntaxHighlighting::Repository m_Repository;
    KSyntaxHighlighting::SyntaxHighlighter *m_pHighlighter;

    CodeEditorSidebar *m_pSideBar;

    bool m_SpaceTabs;
    int32_t m_SpaceIndent;

    bool m_BlockSelection;
    int32_t m_BlockPosition;
    int32_t m_ColumnPosition;

    int32_t m_BlockAnchor;
    int32_t m_ColumnAnchor;

    bool m_DisplayLineNumbers;
    bool m_DisplayFoldingMarkers;

    bool m_FirstTime;
};

class CodeEditorSidebar : public QWidget {
    Q_OBJECT
public:
    explicit CodeEditorSidebar(CodeEditor *editor) :
            QWidget(editor),
            m_pCodeEditor(editor) {
    }
    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(m_pCodeEditor->sidebarWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        m_pCodeEditor->sidebarPaintEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE {
        if (event->x() >= width() - m_pCodeEditor->fontMetrics().lineSpacing()) {
            auto block = m_pCodeEditor->blockAtPosition(event->y());
            if (!block.isValid() || !m_pCodeEditor->isFoldable(block)) {
                return;
            }
            m_pCodeEditor->toggleFold(block);
        }
        QWidget::mouseReleaseEvent(event);
    }

private:
    CodeEditor *m_pCodeEditor;
};

#endif // CODEEDIT_H
