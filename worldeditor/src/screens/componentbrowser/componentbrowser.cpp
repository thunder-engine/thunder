#include "componentbrowser.h"
#include "ui_componentbrowser.h"

#include <QSortFilterProxyModel>

#include "componentmodel.h"

class ComponentFilter : public QSortFilterProxyModel {
public:
    explicit ComponentFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
    }

    void setComponentGroups(const StringList &list) {
        m_list = list;
        invalidate();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;
        if(!m_list.empty()) {
            result = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QString type(model->data(model->index(sourceRow, 1, sourceParent), Qt::DisplayRole).toString());

        for(TString it : m_list) {
            if(type.contains(it.data(), filterCaseSensitivity())) {
                return true;
            }
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 0, sourceParent);

        if(!filterRegularExpression().isValid() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkNameFilter(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegularExpression());
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

protected:
    StringList m_list;

};

ComponentBrowser::ComponentBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ComponentBrowser) {

    ui->setupUi(this);

    m_proxyModel = new ComponentFilter(this);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(0);

    m_proxyModel->setSourceModel(ComponentModel::instance());

    ui->componentsTree->setModel(m_proxyModel);
    ui->componentsTree->setHeaderHidden(true);

    ui->componentsTree->header()->hideSection(1);
    ui->componentsTree->header()->hideSection(2);
}

void ComponentBrowser::setGroups(const StringList &groups) {
    m_proxyModel->setComponentGroups(groups);
    ui->componentsTree->expandAll();
}

void ComponentBrowser::on_findComponent_textChanged(const QString &arg1) {
    m_proxyModel->setFilterFixedString(arg1);
    ui->componentsTree->expandAll();
}

void ComponentBrowser::on_componentsTree_clicked(const QModelIndex &index) {
    if(m_proxyModel->rowCount(index) == 0) {
        QObject *object = static_cast<QObject *>(m_proxyModel->mapToSource(index).internalPointer());
        emit componentSelected(object->objectName());
    }
}

void ComponentBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
