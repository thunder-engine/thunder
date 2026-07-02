#include "editor/asseteditor.h"

#include <QMessageBox>

#include <url.h>
#include <filedialog.h>

#include "editor/projectsettings.h"
#include "editor/assetmanager.h"
#include "editor/undostack.h"

AssetEditor::AssetEditor() :
        m_undoRedo(new UndoStack) {

}

AssetEditor::~AssetEditor() {

}

void AssetEditor::onNewAsset() {
    m_settings.clear();
    m_undoRedo->clear();
}

void AssetEditor::onOpenAsset() {
    FileDialog dialog;

    dialog.setMode(FileDialog::OpenFile);
    dialog.setWindowTitle(TString("Save ") + assetType());

    StringList list;
    for(auto &it : suffixes()) {
        list.push_back(TString("*.") + it);
    }

    dialog.addFilter(assetType(), list);
    dialog.setDirectory(ProjectSettings::instance()->contentPath());

    if(dialog.exec()) {
        TString path(dialog.getSelectedFile());
        if(!path.isEmpty()) {
            loadAsset(AssetManager::instance()->fetchSettings(path));
        }
    }
}

void AssetEditor::loadAsset(AssetConverterSettings *settings) {
    m_settings = { settings };
    m_undoRedo->clear();
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

    cleanModified();
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

StringList AssetEditor::suffixes() const {
    return StringList();
}

std::list<AssetConverterSettings *> &AssetEditor::openedDocuments() {
    return m_settings;
}

StringList AssetEditor::componentGroups() const {
    return StringList();
}

void AssetEditor::cleanModified() const {
    m_undoRedo->setClean();
}

bool AssetEditor::isModified() const {
    return !m_undoRedo->isClean();
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
            cleanModified();
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
    StringList list;
    for(auto &it : suffixes()) {
        list.push_back(TString("*.") + it);
    }

    FileDialog dialog;

    dialog.setMode(FileDialog::SaveFile);
    dialog.setWindowTitle(TString("Save ") + assetType);
    dialog.addFilter(assetType, list);

    dialog.setDirectory(ProjectSettings::instance()->contentPath());

    if(dialog.exec()) {
        TString path(dialog.getSelectedFile());
        Url info(path);
        if(info.suffix().isEmpty()) {
            path += TString(".") + AssetEditor::suffixes().front();
        }
        saveAsset(path);
    }
}

void AssetEditor::onObjectCreate(const TString &type) {
    A_UNUSED(type);
}

void AssetEditor::onObjectsSelected(Object::ObjectList objects, bool force) {
    A_UNUSED(objects);
    A_UNUSED(force);
}

void AssetEditor::onSelectionDeleted() {

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

QMenu *AssetEditor::hierarchyContextMenu(Object *object) {
    A_UNUSED(object);

    return nullptr;
}

void AssetEditor::changeParent(const Object::ObjectList &objects, Object *parent, int position) {
    A_UNUSED(objects);
    A_UNUSED(parent);
    A_UNUSED(position);
}

QWidget *AssetEditor::propertiesWidget() {
    return nullptr;
}

std::list<QWidget *> AssetEditor::propertiesActionWidgets(Object *object, QWidget *parent) const {
    A_UNUSED(object);
    A_UNUSED(parent);

    return std::list<QWidget *>();
}

QMenu *AssetEditor::propertyContextMenu(Object *object, const TString &property) {
    A_UNUSED(object);
    A_UNUSED(property);

    return nullptr;
}

UndoStack *AssetEditor::undoRedo() const {
    return m_undoRedo;
}

VariantMap AssetEditor::saveState() {
    return VariantMap();
}

void AssetEditor::restoreState(const VariantMap &data) {
    A_UNUSED(data);
}

Object::ObjectList AssetEditor::selected() const {
    return Object::ObjectList();
}
