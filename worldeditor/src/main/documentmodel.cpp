#include "documentmodel.h"

#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>

#include "assetmanager.h"

#include <editor/projectmanager.h>
#include <editor/pluginmanager.h>

#include "editors/animationedit/animationedit.h"

DocumentModel::DocumentModel() {
    addEditor(new AnimationEdit);

    for(auto &it : PluginManager::instance()->extensions("editor")) {
        AssetEditor *editor = reinterpret_cast<AssetEditor *>(PluginManager::instance()->getPluginObject(it));
        addEditor(editor);
    }
}

DocumentModel::~DocumentModel() {
    auto it = m_Documents.begin();
    while(it != m_Documents.end()) {
        if(!it.value()->isSingleInstance()) {
            it.value()->deleteLater();
        }
        ++it;
    }
    m_Documents.clear();

    for(auto &it : m_Editors) {
        delete it;
    }
    m_Editors.clear();
}

void DocumentModel::addEditor(AssetEditor *editor) {
    for(auto &it : editor->suffixes()) {
        m_Editors[it] = editor;
    }
    editor->installEventFilter(this);
    connect(editor, &AssetEditor::itemSelected, this, &DocumentModel::itemSelected);
    connect(editor, &AssetEditor::itemUpdated, this, &DocumentModel::itemUpdated);

    connect(editor, &AssetEditor::dropAsset, this, &DocumentModel::onLoadAsset);
}

QString DocumentModel::fileName(AssetEditor *editor) const {
    return m_Documents.key(editor);
}

void DocumentModel::newFile(AssetEditor *editor) {
    if(checkSave(editor)) {
        closeFile(editor);
        editor->newAsset();
    }
}

AssetEditor *DocumentModel::openFile(const QString &path) {
    QFileInfo info(path);
    QDir dir(ProjectManager::instance()->contentPath());
    AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(dir.absoluteFilePath(info.filePath()));

    AssetEditor *editor = nullptr;

    auto it = m_Documents.find(settings->source());
    if(it != m_Documents.end()) {
        AssetEditor *editor = it.value();
        editor->loadAsset(settings);
        return editor;
    }

    auto e = m_Editors.find(info.suffix().toLower());
    if(e != m_Editors.end()) {
        editor = e.value();
        if(!editor->isSingleInstance()) {
            AssetEditor *instance = editor->createInstance();
            instance->installEventFilter(this);
            editor = instance;
        } else { // Single instance! Need to close previous document
            if(checkSave(editor)) {
                closeFile(editor);
            }
        }
    }

    if(editor) {
        editor->loadAsset(settings);
        m_Documents[settings->source()] = editor;
    }

    return editor;
}

void DocumentModel::saveFile(AssetEditor *editor) {
    if(editor) {
        QString path = m_Documents.key(editor);
        if(!path.isEmpty()) {
            editor->saveAsset(path);
        } else {
            saveFileAs(editor);
        }
    }
}

void DocumentModel::saveFileAs(AssetEditor *editor) {
    if(editor) {
        QString dir = ProjectManager::instance()->contentPath();

        QMap<QString, QStringList> dictionary;
        for(auto &it : editor->suffixes()) {
            dictionary[AssetManager::instance()->assetTypeName("." + it)].push_back(it);
        }

        QStringList filter;
        for(auto it = dictionary.begin(); it != dictionary.end(); ++it) {
            QString item = it.key() + " (";
            for(auto &suffix : it.value()) {
                item += "*." + suffix;
            }
            item += ")";
            filter.push_back(item);
        }

        QString path = QFileDialog::getSaveFileName(nullptr,
                                                    tr("Save Document"),
                                                    dir, filter.join(";;"));
        if(!path.isEmpty()) {
            QFileInfo info(path);
            if(info.suffix().isEmpty()) {
                path += "." + dictionary.begin().value().front();
            }
            editor->saveAsset(path);
        }
    }
}

void DocumentModel::closeFile(AssetEditor *editor) {
    for(auto it = m_Documents.begin(); it != m_Documents.end();) {
        if(it.value() == editor) {
            if(!editor->isSingleInstance()) {
                editor->deleteLater();
            }
            it = m_Documents.erase(it);
        } else {
            ++it;
        }
    }
}

void DocumentModel::saveAll() {
    foreach(auto it, m_Documents) {
        if(it->isModified()) {
            saveFile(it);
        }
    }
}

void DocumentModel::onLoadAsset(QString path) {
    openFile(path);
}

bool DocumentModel::checkSave(AssetEditor *editor) {
    if(editor->isModified()) {
        int result = closeAssetDialog();
        if(result == QMessageBox::Cancel) {
            return false;
        } else if(result == QMessageBox::Yes) {
            saveFile(editor);
        } else {
            editor->setModified(false);
        }
    }
    return true;
}

QList<AssetEditor *> DocumentModel::documents() {
    return m_Documents.values();
}

int DocumentModel::closeAssetDialog() {
    QMessageBox msgBox(nullptr);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("The asset has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    return msgBox.exec();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            QString key = m_Documents.keys().at(index.row());
            QFileInfo info(key);
            if(index.column() == 0) {
                return info.fileName();
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

bool DocumentModel::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::Close) {
        AssetEditor *editor = static_cast<AssetEditor *>(obj);
        if(!checkSave(editor)) {
            event->ignore();
        } else {
            closeFile(editor);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}
