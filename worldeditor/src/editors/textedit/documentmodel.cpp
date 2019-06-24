#include "documentmodel.h"

#include <QDebug>

#include <QTextDocument>
#include <QPlainTextDocumentLayout>
#include <QFileInfo>

#include "codeeditor.h"

DocumentModel::DocumentModel() {
    m_Close.load(":/Style/styles/dark/icons/close.png");
}

CodeEditor *DocumentModel::openFile(const QString &path) {
    auto it = m_Documents.find(path);
    if(it != m_Documents.end()) {
        return it.value();
    }

    CodeEditor *editor = new CodeEditor;
    editor->openFile(path);
    m_Documents[path] = editor;

    connect(editor->document(), SIGNAL(modificationChanged(bool)), this, SIGNAL(layoutChanged()));

    emit layoutAboutToBeChanged();
    emit layoutChanged();

    return editor;
}

CodeEditor *DocumentModel::openFile(const QModelIndex &index) {
    return openFile(m_Documents.keys().at(index.row()));
}

void DocumentModel::saveAll() {
    foreach(auto it, m_Documents) {
        if(it->document()->isModified()) {
            it->saveFile();
        }
    }
}

void DocumentModel:: closeFile(const QModelIndex &index) {
    QString key = m_Documents.keys().at(index.row());
    CodeEditor *editor = m_Documents.value(key);
    if(editor && editor->document()->isModified()) {
        editor->saveFile();
    }
    delete editor;
    m_Documents.remove(key);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            QString key = m_Documents.keys().at(index.row());
            QFileInfo info(key);
            if(index.column() == 0) {
                CodeEditor *editor = m_Documents.value(key);
                return info.fileName() + (editor->document()->isModified() ? "*" : "");
            }
        } break;
        case Qt::DecorationRole: {
            if(index.column() == 1) {
                return m_Close;
            }
        } break;
        case Qt::SizeHintRole: {
            if(index.column() == 1) {
                return m_Close.size();
            }
        } break;
        default: break;
    }

    return QVariant();
}

QVariant DocumentModel::headerData(int, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "";
    }
    return QVariant();
}

int DocumentModel::columnCount(const QModelIndex &) const {
    return 2;
}

QModelIndex DocumentModel::index(int row, int column, const QModelIndex &) const {
    if(row >= m_Documents.size()) {
        return QModelIndex();
    }
    return createIndex(row, column, nullptr);
}

QModelIndex DocumentModel::parent(const QModelIndex &) const {
    return QModelIndex();
}

int DocumentModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return m_Documents.size();
}
