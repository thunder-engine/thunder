#include "editor/asseteditor.h"

#include <QMessageBox>
#include <QFileDialog>

#include <url.h>

#include "editor/projectsettings.h"
#include "editor/undostack.h"

AssetEditor::AssetEditor() :
        m_undoRedo(new UndoStack) {

}
AssetEditor::~AssetEditor() {

}

void AssetEditor::onNewAsset() {
    m_settings.clear();
}

void AssetEditor::loadAsset(AssetConverterSettings *settings) {
    m_settings = { settings };
}

void AssetEditor::loadData(const Variant &data, const TString &suffix) {
    Q_UNUSED(data)
    Q_UNUSED(suffix)
}

bool AssetEditor::allowSaveAs() const {
    return true;
}

void AssetEditor::saveAsset(const TString &path) {
    Q_UNUSED(path)
}

bool AssetEditor::isSingleInstance() const {
    return true;
}

bool AssetEditor::isCopyActionAvailable() const {
    return false;
}

bool AssetEditor::isPasteActionAvailable() const {
    return false;
}

AssetEditor *AssetEditor::createInstance() {
    return nullptr;
}

std::list<AssetConverterSettings *> &AssetEditor::openedDocuments() {
    return m_settings;
}

StringList AssetEditor::componentGroups() const {
    return StringList();
}

void AssetEditor::setModified(bool flag) {
    Q_UNUSED(flag)
}

void AssetEditor::onActivated() {

}

void AssetEditor::onCutAction() {

}

void AssetEditor::onCopyAction() {

}

void AssetEditor::onPasteAction() {

}

int AssetEditor::closeAssetDialog() {
    QMessageBox msgBox(nullptr);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("The asset has been modified."));
    msgBox.setInformativeText(tr("Do you want to save your changes?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    return msgBox.exec();
}

bool AssetEditor::checkSave() {
    if(isModified()) {
        int result = closeAssetDialog();
        if(result == QMessageBox::Cancel) {
            return false;
        } else if(result == QMessageBox::Yes) {
            onSave();
        } else {
            setModified(false);
        }
    }
    return true;
}

void AssetEditor::onSave() {
    if(!m_settings.empty()) {
        if(!m_settings.front()->source().isEmpty()) {
            saveAsset(m_settings.front()->source());
        } else if(allowSaveAs()) {
            onSaveAs();
        }
    }
}

void AssetEditor::onSaveAs() {
    if(m_settings.empty()) {
        return;
    }

    TString assetType(m_settings.front()->typeName());

    std::map<TString, StringList> dictionary;
    for(auto &it : suffixes()) {
        dictionary[assetType].push_back(it);
    }

    StringList filter;
    for(auto it : dictionary) {
        TString item = it.first + " (";
        for(auto &suffix : it.second) {
            item += TString("*.") + suffix;
        }
        item += ")";
        filter.push_back(item);
    }

    QString path(QFileDialog::getSaveFileName(nullptr,
                                              QString("Save ") + assetType.data(),
                                              ProjectSettings::instance()->contentPath().data(), TString::join(filter, ";;").data()));
    if(!path.isEmpty()) {
        Url info(path.toStdString());
        if(info.suffix().isEmpty()) {
            path += QString(".") + dictionary.begin()->second.front().data();
        }
        saveAsset(path.toStdString());
    }
}

void AssetEditor::onObjectCreate(TString type) {
    A_UNUSED(type);
}

void AssetEditor::onObjectsSelected(Object::ObjectList objects, bool force) {
    A_UNUSED(objects);
    A_UNUSED(force);
}

void AssetEditor::onObjectsDeleted(Object::ObjectList objects) {
    A_UNUSED(objects);
}

void AssetEditor::onUpdated() {

}

void AssetEditor::onDrop(QDropEvent *event) {
    A_UNUSED(event);
}
void AssetEditor::onDragEnter(QDragEnterEvent *event) {
    A_UNUSED(event);
}
void AssetEditor::onDragMove(QDragMoveEvent *event) {
    A_UNUSED(event);
}
void AssetEditor::onDragLeave(QDragLeaveEvent *event) {
    A_UNUSED(event);
}

void AssetEditor::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    A_UNUSED(objects);
    A_UNUSED(property);
    A_UNUSED(value);
}

QMenu *AssetEditor::objectContextMenu(Object *object) {
    A_UNUSED(object);

    return nullptr;
}

QWidget *AssetEditor::propertiesWidget(QWidget *parent) {
    return nullptr;
}

UndoStack *AssetEditor::undoRedo() const {
    return m_undoRedo;
}

std::list<QWidget *> AssetEditor::createActionWidgets(Object *object, QWidget *parent) const {
    return std::list<QWidget *>();
}

VariantMap AssetEditor::saveState() {
    return VariantMap();
}

void AssetEditor::restoreState(const VariantMap &data) {
    A_UNUSED(data);
}
