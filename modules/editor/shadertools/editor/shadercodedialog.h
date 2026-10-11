#ifndef SHADERCODEDIALOG_H
#define SHADERCODEDIALOG_H

#include <QDialog>

#include <variant.h>

class LegacyAssetEditor;
class QPlainTextEdit;

namespace Ui {
    class ShaderCodeDialog;
}

class ShaderCodeDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShaderCodeDialog(QWidget *parent = nullptr);
    ~ShaderCodeDialog();

    void setData(const VariantMap &data);

private slots:
    void showShader();

private:
    Ui::ShaderCodeDialog *ui;

    LegacyAssetEditor *m_codeEditor = nullptr;

    QPlainTextEdit *m_plainText = nullptr;

    VariantMap m_data;

};

#endif // SHADERCODEDIALOG_H
