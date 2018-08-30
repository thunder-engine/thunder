#ifndef CONTENTSELECT_H
#define CONTENTSELECT_H

#include <QWidget>

#include <stdint.h>

class QSortFilterProxyModel;
class ContentBrowser;

class IConverterSettings;

namespace Ui {
    class ContentSelect;
}

class ContentSelect : public QWidget {
    Q_OBJECT

public:
    explicit ContentSelect      (QWidget *parent = 0);
    ~ContentSelect              ();

    QString                     data                            () const;

    void                        setType                         (const uint8_t type);
    void                        setData                         (const QString &guid);

signals:
    void                        assetChanged                    (IConverterSettings *settings);

private slots:
    void                        onAssetSelected                 (IConverterSettings *settings);

private:
    Ui::ContentSelect          *ui;

    ContentBrowser             *m_pBrowser;
};

#endif // CONTENTSELECT_H
