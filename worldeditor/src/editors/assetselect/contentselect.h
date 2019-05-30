#ifndef CONTENTSELECT_H
#define CONTENTSELECT_H

#include <QWidget>

#include <stdint.h>

class QSortFilterProxyModel;
class AssetBrowser;

class IConverterSettings;

namespace Ui {
    class ContentSelect;
}

class ContentSelect : public QWidget {
    Q_OBJECT

public:
    explicit ContentSelect      (QWidget *parent = nullptr);
    ~ContentSelect              ();

    QString                     data                            () const;

    void                        setType                         (const int32_t type);
    void                        setData                         (const QString &guid);

signals:
    void                        assetChanged                    (IConverterSettings *settings);

private slots:
    void                        onAssetSelected                 (IConverterSettings *settings);

private:
    Ui::ContentSelect *ui;

    AssetBrowser *m_pBrowser;

    QString m_Guid;
};

#endif // CONTENTSELECT_H
