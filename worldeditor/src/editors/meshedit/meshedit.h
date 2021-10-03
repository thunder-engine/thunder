#ifndef MESHEDIT_H
#define MESHEDIT_H

#include <editor/asseteditor.h>

class Actor;

namespace Ui {
    class MeshEdit;
}

class MeshEdit : public AssetEditor {
    Q_OBJECT

public:
    MeshEdit();
    ~MeshEdit();

private:
    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const QString &path) override;
    bool isModified() const override;

    QStringList suffixes() const override;

    void changeEvent(QEvent *event) override;

    Ui::MeshEdit *ui;

    Actor *m_pMesh;
    Actor *m_pGround;
    Actor *m_pLight;

private slots:
    void onUpdateTemplate();

};

#endif // MESHEDIT_H
