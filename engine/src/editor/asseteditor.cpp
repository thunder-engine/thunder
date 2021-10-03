#include "editor/asseteditor.h"

AssetEditor::AssetEditor() :
    m_pSettings(nullptr) {

}
AssetEditor::~AssetEditor() {

}

void AssetEditor::newAsset() {
    m_pSettings = nullptr;
}

void AssetEditor::loadAsset(AssetConverterSettings *settings) {
    m_pSettings = settings;
}

void AssetEditor::saveAsset(const QString &path) {
    Q_UNUSED(path)
}

bool AssetEditor::isSingleInstance() const {
    return true;
}

AssetEditor *AssetEditor::createInstance() {
    return nullptr;
}

void AssetEditor::setModified(bool flag) {
    Q_UNUSED(flag)
}
