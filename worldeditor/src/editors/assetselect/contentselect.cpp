#include "contentselect.h"
#include "ui_contentselect.h"

#include <QFileInfo>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QWidgetAction>
#include <QMenu>

#include "assetlist.h"
#include "assetbrowser.h"

#include "assetmanager.h"

ContentSelect::ContentSelect(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ContentSelect) {

    ui->setupUi(this);

    QMenu *assetMenu = new QMenu();
    ui->toolButton->setMenu(assetMenu);

    //assetMenu->addAction(new QAction(tr("Copy"), assetMenu));

    m_pBrowser  = new AssetBrowser(this);
    connect(m_pBrowser, &AssetBrowser::assetSelected, this, &ContentSelect::onAssetSelected);
    connect(m_pBrowser, &AssetBrowser::assetSelected, this, &ContentSelect::assetChanged);

    QWidgetAction *action = new QWidgetAction(assetMenu);
    action->setDefaultWidget(m_pBrowser);
    assetMenu->addAction(action);
}

ContentSelect::~ContentSelect() {
    delete ui;
}

QString ContentSelect::data() const {
    return m_Guid;
}

void ContentSelect::setData(const QString &guid) {
    if(!guid.isEmpty()) {
        if(m_Guid != guid) {
            string path = AssetManager::instance()->guidToPath(guid.toStdString());
            if(!path.empty()) {
                QFileInfo file(path.c_str());
                m_pBrowser->setSelected(file.filePath());
                ui->toolButton->setText(QString(" ") + file.baseName());
                QImage img  = m_pBrowser->icon(file.filePath());
                if(!img.isNull()) {
                    ui->toolButton->setIcon(QPixmap::fromImage(img.scaled(ui->toolButton->iconSize())));
                }
            } else {
                ui->toolButton->setText("Ivalid");
            }
            m_Guid = guid;
        }
    } else {
        ui->toolButton->setText("None");
    }
}

void ContentSelect::setType(const QString &type) {
    m_pBrowser->filterByType(type);
}

void ContentSelect::onAssetSelected(const QString &uuid) {
    setData(uuid);
    ui->toolButton->menu()->hide();
}
