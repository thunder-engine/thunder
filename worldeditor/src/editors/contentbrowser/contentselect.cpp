#include "contentselect.h"
#include "ui_contentselect.h"

#include <QFileInfo>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QWidgetAction>
#include <QMenu>

#include "contentlist.h"
#include "contentbrowser.h"

#include "assetmanager.h"

ContentSelect::ContentSelect(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ContentSelect) {

    ui->setupUi(this);

    QMenu *assetMenu    = new QMenu();
    ui->toolButton->setMenu(assetMenu);

    //assetMenu->addAction(new QAction(tr("Copy"), assetMenu));

    m_pBrowser  = new ContentBrowser(this);
    m_pBrowser->setCompact(true);
    connect(m_pBrowser, SIGNAL(assetSelected(IConverterSettings *)), this, SLOT(onAssetSelected(IConverterSettings *)));
    connect(m_pBrowser, SIGNAL(assetSelected(IConverterSettings *)), this, SIGNAL(assetChanged(IConverterSettings *)));

    QWidgetAction *action   = new QWidgetAction(assetMenu);
    action->setDefaultWidget(m_pBrowser);
    assetMenu->addAction(action);
}

ContentSelect::~ContentSelect() {
    delete ui;
}

QString ContentSelect::data() const {
    return QString();
}

void ContentSelect::setData(const QString &guid) {
    if(!guid.isEmpty()) {
        string path = AssetManager::instance()->guidToPath(guid.toStdString());
        if(!path.empty()) {
            QFileInfo file(path.c_str());
            m_pBrowser->setSelected(file.filePath());
            ui->toolButton->setText(file.baseName());
            QImage img  = m_pBrowser->icon(file.filePath());
            if(!img.isNull()) {
                ui->label->setPixmap( QPixmap::fromImage(img.scaled( ui->label->size() )) );
            }
        } else {
            ui->toolButton->setText("Ivalid");
        }
    } else {
        ui->toolButton->setText("None");
    }
}

void ContentSelect::setType(const uint8_t type) {
    m_pBrowser->filterByType(type);
}

void ContentSelect::onAssetSelected(IConverterSettings *settings) {
    setData(settings->destination());
    ui->toolButton->menu()->hide();
}
