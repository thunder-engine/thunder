#ifndef LEGACYPIPELINEEDIT_H
#define LEGACYPIPELINEEDIT_H

#include <editor/legacyasseteditor.h>

class PipelineConverter;
class PipelineTaskGraph;
class LegacyCameraController;

class UndoCommand;
class PipelineProxy;

namespace Ui {
    class LegacyPipelineEdit;
}

class LegacyPipelineEdit : public LegacyAssetEditor {
    Q_OBJECT
    
public:
    LegacyPipelineEdit();
    ~LegacyPipelineEdit();

    void onGraphUpdated();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onObjectsChanged(const TString &property, const Variant &value) override;

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void readSettings();
    void writeSettings();

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path) override;

    void changeEvent(QEvent *event) override;

    StringList suffixes() const override;
    TString assetType() const override;

    Object::ObjectList selected() const override;

private:
    Ui::LegacyPipelineEdit *ui;

    PipelineTaskGraph *m_graph;
    PipelineConverter *m_builder;

    LegacyCameraController *m_controller;

    PipelineProxy *m_proxy;

};

#endif // LEGACYPIPELINEEDIT_H
