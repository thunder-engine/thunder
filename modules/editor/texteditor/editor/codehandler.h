#ifndef CODHANDLER_H
#define CODHANDLER_H

#include <QObject>

#include <QPlainTextEdit>

class TextWidget;

class CodeHandler : public QObject {
    Q_OBJECT
public:
    explicit CodeHandler(TextWidget *widget);

private:
    friend class CodeEditSidebar;

    bool eventFilter(QObject *obj, QEvent *event) override;
    bool keyPress(QKeyEvent *event);

    QString indentationString(int startColumn, int targetColumn, int padding, const QTextBlock &block) const;
    int lineIndentPosition(const QString &text) const;
    int spacesLeftFromPosition(const QString &text, int position) const;
    int indentedColumn(int column, bool doIndent) const;

    void commentSelection();
    void indentSelection();

    QString copyBlockSelection();
    void removeBlockSelection();
    void insertIntoBlockSelection(const QString &text);

    int32_t firstNonIndent(const QString &text) const;

    void setCursorToColumn(QTextCursor &cursor, int column, QTextCursor::MoveMode moveMode = QTextCursor::MoveAnchor);

private:
    TextWidget *m_widget;

};

#endif // CODEHANDLER_H
