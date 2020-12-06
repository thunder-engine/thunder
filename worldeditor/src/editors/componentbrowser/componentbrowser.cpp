#include "componentbrowser.h"
#include "ui_componentbrowser.h"

#include <QDrag>
#include <QMimeData>
#include <QSortFilterProxyModel>

#include "componentmodel.h"

class ComponentFilter : public QSortFilterProxyModel {
public:
    explicit ComponentFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
    }

    void setComponentGroups(const QStringList &list) {
        m_List      = list;
        invalidate();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;
        if(!m_List.isEmpty()) {
            result  = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result     &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index   = sourceModel()->index(sourceRow, 1, sourceParent);
        foreach(QString it, m_List) {
            if(sourceModel()->data(index, Qt::DisplayRole).toString().contains(it, filterCaseSensitivity())) {
                return true;
            }
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model   = sourceModel();
        QModelIndex index           = model->index(sourceRow, 0, sourceParent);
        if(!filterRegExp().isEmpty() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkNameFilter(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegExp());
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
protected:
    QStringList     m_List;
};

ComponentBrowser::ComponentBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ComponentBrowser) {

    ui->setupUi(this);

    m_pProxyModel   = new ComponentFilter(this);
    m_pProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pProxyModel->sort(0);

    connect(ui->componentsTree, SIGNAL(dragStarted(Qt::DropActions)), this, SLOT(onDragStarted(Qt::DropActions)));

    m_pProxyModel->setSourceModel(ComponentModel::instance());

    ui->componentsTree->setModel(m_pProxyModel);
    ui->componentsTree->setHeaderHidden(true);

    ui->componentsTree->header()->hideSection(1);
    ui->componentsTree->header()->hideSection(2);
}

void ComponentBrowser::setGroups(const QStringList &groups) {
    m_pProxyModel->setComponentGroups(groups);
    ui->componentsTree->expandAll();
}

void ComponentBrowser::setModel(QAbstractItemModel *model) {
    m_pProxyModel->setSourceModel(model);
}

void ComponentBrowser::onDragStarted(Qt::DropActions supportedActions) {
    QModelIndex index   = m_pProxyModel->mapToSource(ui->componentsTree->selectionModel()->selectedIndexes().front());

    QMimeData *data     = new QMimeData;
    data->setData(gMimeComponent, qPrintable(static_cast<QObject *>(index.internalPointer())->objectName()) );

    QDrag *drag         = new QDrag(this);
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::CopyAction);
}

void ComponentBrowser::on_findComponent_textChanged(const QString &arg1) {
    m_pProxyModel->setFilterFixedString(arg1);
    ui->componentsTree->expandAll();
}

void ComponentBrowser::on_componentsTree_clicked(const QModelIndex &index) {
    if(m_pProxyModel->rowCount(index) == 0) {
        QObject *object = static_cast<QObject *>(m_pProxyModel->mapToSource(index).internalPointer());
        emit componentSelected(object->objectName());
    }
}

void ComponentBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
