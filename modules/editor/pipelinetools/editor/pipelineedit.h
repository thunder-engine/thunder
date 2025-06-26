#ifndef PIPELINEEDIT_H
#define PIPELINEEDIT_H

#include <editor/asseteditor.h>

class PipelineConverter;
class PipelineTaskGraph;
class CameraController;

class UndoCommand;

namespace Ui {
    class PipelineEdit;
}

class PipelineEdit : public AssetEditor {
    Q_OBJECT
    
public:
    PipelineEdit();
    ~PipelineEdit();

private slots:
    void onCutAction() override;
    void onCopyAction() override;
    void onPasteAction() override;

    void onActivated() override;

    void onGraphUpdated();

    void onObjectsChanged(const std::list<Object *> &objects, QString property, const Variant &value) override;

private:
    bool isCopyActionAvailable() const override;
    bool isPasteActionAvailable() const override;

    void readSettings();
    void writeSettings();

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;

    void changeEvent(QEvent *event) override;

    bool isModified() const override;

    QStringList suffixes() const override;

private:
    Ui::PipelineEdit *ui;

    PipelineTaskGraph *m_graph;
    PipelineConverter *m_builder;

    CameraController *m_controller;

    const UndoCommand *m_lastCommand;

};

#endif // PIPELINEEDIT_H
