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
    void                        assetChanged                    (const QString &uuid);

private slots:
    void                        onAssetSelected                 (const QString &uuid);

private:
    Ui::ContentSelect *ui;

    AssetBrowser *m_pBrowser;

    QString m_Guid;
};

#endif // CONTENTSELECT_H
