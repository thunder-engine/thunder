#include "contentbrowser.h"
#include "ui_contentbrowser.h"

#include <QMetaEnum>
#include <QMenu>
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSettings>
#include <QWidgetAction>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>

#include "common.h"

#include "contentlist.h"
#include "contenttree.h"

#include "assetmanager.h"
#include "projectmanager.h"

const QString gTemplateName("${templateName}");

class ContentFilter : public QSortFilterProxyModel {
public:
    typedef QList<uint8_t>  TypeList;

    explicit ContentFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {

        sort(0);
    }

    void setContentTypes(const TypeList &list) {
        m_List      = list;
        invalidate();
    }

    void setRootPath(const QString &path) {
        ContentList::instance()->setRootPath(path);
        invalidate();
    }

    QString rootPath() const {
        return ContentList::instance()->rootPath();
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
        bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

        ContentList *inst   = ContentList::instance();
        bool isFile1    = inst->data(inst->index(left.row(), 2, left.parent()), Qt::DisplayRole).toBool();
        bool isFile2    = inst->data(inst->index(right.row(), 2, right.parent()), Qt::DisplayRole).toBool();
        if(!isFile1 && isFile2) {
            return asc;
        }
        if(isFile1 && !isFile2) {
            return !asc;
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = checkRootPath(sourceRow, sourceParent);
        if(!m_List.isEmpty()) {
            result  = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result     &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index   = sourceModel()->index(sourceRow, 3, sourceParent);
        foreach (uint8_t it, m_List) {
            uint8_t type    = sourceModel()->data(index).toInt();
            if(it == type) {
                return true;
            }
        }
        return false;
    }

    bool checkRootPath(int sourceRow, const QModelIndex &sourceParent) const {
        return (rootPath() == sourceModel()->data(sourceModel()->index(sourceRow, 1, sourceParent)).toString());
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index   = sourceModel()->index(sourceRow, 2, sourceParent);
        return (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) && (filterRegExp().isEmpty() || sourceModel()->data(index).toBool()));
    }

    TypeList    m_List;
};

class AssetItemDeligate : public QStyledItemDelegate  {
public:
    explicit AssetItemDeligate(QObject *parent = 0) :
            QStyledItemDelegate(parent),
            m_Scale(1.0f) {
    }


    float itemScale() const {
        return m_Scale;
    }

    void setItemScale(float scale) {
        m_Scale = scale;

    }

private:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        if(QStyleOptionViewItemV4 *v4 = qstyleoption_cast<QStyleOptionViewItemV4 *>(option)) {
            QVariant value  = index.data(Qt::DecorationRole);
            switch(value.type()) {
                case QVariant::Image: {
                    QImage image    = value.value<QImage>();
                    if(!image.isNull()) {
                        QSize origin    = image.size();
                        image           = image.scaled(origin.width() * m_Scale, origin.height() * m_Scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        v4->icon        = QIcon(QPixmap::fromImage(image));
                        v4->decorationSize = image.size();
                    }
                } break;
                default: break;
            }
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        return QStyledItemDelegate::sizeHint(option, index) * m_Scale;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        return new QLineEdit(parent);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        QRect r = editor->geometry();

        r.setX(option.rect.x());
        r.setWidth(option.rect.width());
        editor->setGeometry(r);

        QLineEdit *e    = dynamic_cast<QLineEdit *>(editor);
        if(e) {
            e->setAlignment(Qt::AlignHCenter);
            e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const {
        uint32_t width  = editor->width();
        QStyledItemDelegate::setEditorData(editor, index);
        editor->setFixedWidth(width);
    }

    float       m_Scale;
};

ContentBrowser::ContentBrowser(QWidget* parent) :
        QWidget(parent),
        ui(new Ui::ContentBrowser),
        m_pSelected(nullptr) {

    ui->setupUi(this);

    m_pContentDeligate  = new AssetItemDeligate();
    m_pContentProxy     = new ContentFilter(this);
    m_pContentProxy->setSourceModel(ContentList::instance());

    m_pContentDeligate->setItemScale(0.75f);

    ui->contentList->setItemDelegate(m_pContentDeligate);
    ui->contentList->setModel(m_pContentProxy);

    m_pTreeProxy = new QSortFilterProxyModel(this);
    m_pTreeProxy->setSourceModel(new ContentTree(this));
    m_pTreeProxy->sort(0);

    ui->contentTree->setModel(m_pTreeProxy);
    ui->contentTree->expandToDepth(1);

    m_pFilterMenu   = new QMenu(this);
    for(int i = IConverter::ContentText; i < IConverter::ContentLast; i++) {
        QAction *a  = new QAction(QMetaEnum::fromType<ContentTypes>().valueToKey(i), m_pFilterMenu);
        a->setCheckable(true);
        m_pFilterMenu->addAction(a);
    }
    ui->filterButton->setMenu(m_pFilterMenu);
    connect(m_pFilterMenu, SIGNAL(triggered(QAction*)), this, SLOT(onFilterMenuTriggered(QAction*)));

    readSettings();

    setCompact(false);
    on_contentTree_clicked(QModelIndex());

    QString showIn(tr("Show in Explorer"));

    {
        QLabel *label = new QLabel(tr("Create Asset"), this);
        QWidgetAction *a = new QWidgetAction(&m_CreationMenu);
        a->setDefaultWidget(label);

        m_CreationMenu.addAction(tr("New Folder"))->setData(true);
        m_CreationMenu.addAction(showIn, this, SLOT(showInGraphicalShell()));
        m_CreationMenu.addSeparator();
        m_CreationMenu.addAction(a);
        m_CreationMenu.addAction(tr("Material"))->setData(".mtl");
        m_CreationMenu.addAction(tr("NativeBehaviour"))->setData(".cpp");
    }

    createAction(showIn, SLOT(showInGraphicalShell()));
    createAction(tr("Duplicate"), SLOT(onItemDuplicate()));
    createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2));
    createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete));
    m_ContentMenu.addSeparator();
    createAction(tr("Reimport"), SLOT(onItemReimport()));

    connect(&m_CreationMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCreationMenuTriggered(QAction*)));
}

ContentBrowser::~ContentBrowser() {
    writeSettings();

    delete ui;

    delete m_pSelected;
}

void ContentBrowser::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value  = settings.value("content.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void ContentBrowser::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("content.geometry", ui->splitter->saveState());
}

void ContentBrowser::onCreationMenuTriggered(QAction *action) {
    QDir dir(m_pContentProxy->rootPath());
    switch(action->data().type()) {
        case QVariant::Bool: {
            QString name    = "NewFolder";
            AssetManager::findFreeName(name, dir.path());
            dir.mkdir(name);
        } break;
        case QVariant::String: {
            QString name    = QString("New") + action->text();
            QString suff    = action->data().toString();
            AssetManager::findFreeName(name, dir.path(), suff);

            QFile file(ProjectManager::instance()->templatePath() + "/" + suff + ".tpl");
            if(file.open(QFile::ReadOnly | QFile::Text)) {
                QByteArray data(file.readAll());
                file.close();

                data.replace(gTemplateName, qPrintable(name));

                QFile gen(dir.path() + QDir::separator() + name + suff);
                if(gen.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
                    gen.write(data);
                    gen.close();
                }
            }
        } break;
        default: break;
    }
}

void ContentBrowser::onFilterMenuTriggered(QAction *) {
    ContentFilter::TypeList list;
    foreach (QAction *it, m_pFilterMenu->findChildren<QAction *>()) {
        if(it->isChecked()) {
            list.append(QMetaEnum::fromType<ContentTypes>().keyToValue(qPrintable(it->text())));
        }
    }
    m_pContentProxy->setContentTypes(list);
}

void ContentBrowser::onItemRename() {
    ui->contentList->edit(ui->contentList->currentIndex());
}

void ContentBrowser::onItemDuplicate() {
    QModelIndex index   = m_pContentProxy->mapToSource(ui->contentList->currentIndex());
    QString path    = ContentList::instance()->path(index);
    AssetManager::instance()->duplicateResource(QFileInfo(path));
}

void ContentBrowser::onItemReimport() {
    QModelIndex index   = m_pContentProxy->mapToSource(ui->contentList->currentIndex());
    ContentList::instance()->reimportResource(index);
}

void ContentBrowser::onItemDelete() {
    QMessageBox msgBox(QMessageBox::Question, tr("Delete Asset"), tr("This action cannot be reverted. Do you want to delete selected asset?"), QMessageBox::Yes | QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes) {
        ContentList::instance()->removeResource(m_pContentProxy->mapToSource(ui->contentList->currentIndex()));
    }
}

void ContentBrowser::setCompact(bool value) {
    m_Compact   = value;
    ui->contentTree->setVisible(!value);
    ui->filterButton->setVisible(!value);
    ui->contentList->setViewMode((value) ? QListView::ListMode : QListView::IconMode);
}

void ContentBrowser::filterByType(const uint8_t type) {
    m_pContentProxy->setContentTypes({ AssetManager::instance()->toContentType(type) });
}

void ContentBrowser::setSelected(const QString &resource) {
    ui->contentList->setCurrentIndex( m_pContentProxy->mapFromSource(ContentList::instance()->findResource(resource)) );
}

QImage ContentBrowser::icon(const QString &resource) const {
    return ContentList::instance()->icon(ContentList::instance()->findResource(resource));
}

void ContentBrowser::on_findContent_textChanged(const QString &arg1) {
    m_pContentProxy->setFilterFixedString(arg1);
}

void ContentBrowser::on_contentTree_clicked(const QModelIndex &index) {
    ui->findContent->clear();
    m_pContentProxy->setRootPath(static_cast<ContentTree *>(m_pTreeProxy->sourceModel())->dirPath(m_pTreeProxy->mapToSource(index)));
}

void ContentBrowser::on_contentList_clicked(const QModelIndex &index) {
    QModelIndex origin   = m_pContentProxy->mapToSource(index);

    QFileInfo path  = ProjectManager::instance()->contentPath() + QDir::separator() + ContentList::instance()->path(origin);
    if(path.isFile()) {
        if(m_pSelected) {
            delete m_pSelected;
        }
        m_pSelected = AssetManager::instance()->createSettings(path);
        emit assetSelected(m_pSelected);
    }
}

void ContentBrowser::on_contentList_doubleClicked(const QModelIndex &index) {
    const QModelIndex origin    = m_pContentProxy->mapToSource(index);

    ContentList *inst   = ContentList::instance();
    if(inst->isDir(origin)) {
        QObject *item   = static_cast<QObject *>(origin.internalPointer());
        QFileInfo info(ProjectManager::instance()->contentPath() + QDir::separator() + item->objectName());

        m_pContentProxy->setRootPath( info.absoluteFilePath() );
    } else {
        if(!m_Compact) {
            QObject *sender = AssetManager::instance()->openEditor(inst->path(origin));
            connect(sender, SIGNAL(templateUpdate()), ui->contentList, SLOT(update()));
        }
    }

}

void ContentBrowser::createAction(const QString &name, const char *member, const QKeySequence &shortcut) {
    QAction *a  = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), this, member);
    ui->contentList->addAction(a);
    m_ContentMenu.addAction(a);
}

void ContentBrowser::on_contentList_customContextMenuRequested(const QPoint &pos) {
    QWidget *w  = static_cast<QWidget*>(QObject::sender());
    if(ui->contentList->selectionModel()->selectedIndexes().empty()) {
        m_CreationMenu.exec(w->mapToGlobal(pos));
    } else {
        m_ContentMenu.exec(w->mapToGlobal(pos));
    }
}

void ContentBrowser::showInGraphicalShell() {
    //QDesktopServices::openUrl(QUrl("file:///D:/", QUrl::TolerantMode));

    QString path;
    QModelIndexList list    = ui->contentList->selectionModel()->selectedIndexes();
    if(list.empty()) {
        path    = m_pContentProxy->rootPath();
    } else {
        const QModelIndex origin    = m_pContentProxy->mapToSource(list.first());
        QObject *item   = static_cast<QObject *>(origin.internalPointer());
        if(item) {
            QFileInfo info(ProjectManager::instance()->contentPath() + QDir::separator() + item->objectName());
            path    = info.absoluteFilePath();
        }
    }

#if defined(Q_OS_WIN)
    QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(path));
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(path);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
/*
    // we cannot select a file here, because no file browser really supports it...
    const QFileInfo fileInfo(path);
    const QString folder = fileInfo.absoluteFilePath();
    const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
    QProcess browserProc;
    const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
    if (debug) {
        qDebug() <<  browserArgs;
    }
    bool success = browserProc.startDetached(browserArgs);
    const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
    success = success && error.isEmpty();
    if (!success) {
        showGraphicalShellError(parent, app, error);
    }
*/
#endif
}
