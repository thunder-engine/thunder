#ifndef PIPELINEEDIT_H
#define PIPELINEEDIT_H

#include <editor/legacyasseteditor.h>

class PipelineConverter;
class PipelineTaskGraph;
class LegacyCameraController;

class UndoCommand;
class PipelineProxy;

namespace Ui {
    class PipelineEdit;
}

class PipelineEdit : public LegacyAssetEditor {
    Q_OBJECT
    
public:
    PipelineEdit();
    ~PipelineEdit();

    void onGraphUpdated();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;

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
    Ui::PipelineEdit *ui;

    PipelineTaskGraph *m_graph;
    PipelineConverter *m_builder;

    LegacyCameraController *m_controller;

    PipelineProxy *m_proxy;

};

#endif // PIPELINEEDIT_H
