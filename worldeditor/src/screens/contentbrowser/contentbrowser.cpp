#include "contentbrowser.h"
#include "ui_contentbrowser.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSettings>
#include <QWidgetAction>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

#include <global.h>

#include "contenttree.h"
#include "commitrevert.h"

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>
#include <editor/codebuilder.h>

#include "../propertyedit/propertymodel.h"

#define ICON_SIZE 128

class ContentItemDeligate : public QStyledItemDelegate  {
public:
    explicit ContentItemDeligate(QObject *parent = nullptr) :
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
        QVariant value = index.data(Qt::DecorationRole);
        switch(value.type()) {
            case QVariant::Image: {
                QImage image = value.value<QImage>();
                if(!image.isNull()) {
                    image = image.scaled(image.size() * m_Scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    option->icon = QIcon(QPixmap::fromImage(image));
                    option->decorationSize = image.size();
                }
            } break;
            default: break;
        }
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
        return new QLineEdit(parent);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        QRect r = editor->geometry();

        r.setX(option.rect.x());
        r.setWidth(option.rect.width());
        editor->setGeometry(r);

        QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
        if(e) {
            e->setAlignment(Qt::AlignHCenter);
            e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const {
        QStyledItemDelegate::setEditorData(editor, index);
        editor->setFixedWidth(editor->width());
    }

    float m_Scale;
};

ContentBrowser::ContentBrowser(QWidget* parent) :
        QWidget(parent),
        ui(new Ui::ContentBrowser),
        m_commitRevert(nullptr),
        m_settings(nullptr) {

    ui->setupUi(this);

    ContentItemDeligate *treeDeligate = new ContentItemDeligate;
    treeDeligate->setItemScale(20.0f / ICON_SIZE);

    m_treeProxy = new ContentTreeFilter(this);
    m_treeProxy->setSourceModel(ContentTree::instance());
    m_treeProxy->setContentTypes({0});
    m_treeProxy->sort(0);

    ui->contentTree->setItemDelegate(treeDeligate);
    ui->contentTree->setModel(m_treeProxy);
    ui->contentTree->expandToDepth(1);

    // Content list
    m_contentDeligate = new ContentItemDeligate;
    m_contentDeligate->setItemScale(0.75f);

    m_listProxy = new ContentTreeFilter(this);
    m_listProxy->setSourceModel(ContentTree::instance());
    m_listProxy->sort(0);

    ui->contentList->setItemDelegate(m_contentDeligate);
    ui->contentList->setModel(m_listProxy);

    ui->contentList->setRootIndex(m_listProxy->mapFromSource(ContentTree::instance()->getContent()));

    connect(ContentTree::instance(), &ContentTree::layoutChanged, m_treeProxy, &ContentTreeFilter::invalidate);

    m_filterMenu = new QMenu(this);

    ui->filterButton->setMenu(m_filterMenu);
    connect(m_filterMenu, &QMenu::triggered, this, &ContentBrowser::onFilterMenuTriggered);
    connect(m_filterMenu, &QMenu::aboutToShow, this, &ContentBrowser::onFilterMenuAboutToShow);

    readSettings();
}

ContentBrowser::~ContentBrowser() {
    writeSettings();

    delete ui;
}

void ContentBrowser::readSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value("content.geometry");
    if(value.isValid()) {
        ui->splitter->restoreState(value.toByteArray());
    }
}

void ContentBrowser::writeSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("content.geometry", ui->splitter->saveState());
}

QWidget *ContentBrowser::commitRevert() {
    if(m_commitRevert == nullptr) {
        m_commitRevert = new CommitRevert();
        if(m_settings) {
            m_commitRevert->setObject(m_settings);
        }
    }
    return m_commitRevert;
}

void ContentBrowser::createContextMenus() {
    QString showIn(tr("Show in Explorer"));
    QString newFolder(tr("New Folder"));
    QLabel *label = new QLabel(tr("Create Asset"), this);
    QWidgetAction *a = new QWidgetAction(&m_creationMenu);
    a->setDefaultWidget(label);

    m_creationMenu.addAction(newFolder)->setData(true);
    m_creationMenu.addAction(showIn, this, SLOT(showInGraphicalShell()));
    m_creationMenu.addAction(tr("Import New Asset..."), this, SLOT(importAsset()));
    m_creationMenu.addSeparator();
    m_creationMenu.addAction(a);

    QStringList paths;
    foreach(auto it, AssetManager::instance()->builders()) {
        QString path(it->templatePath());
        if(!path.isEmpty()) {
            paths.push_back(path);
        }
    }

    foreach(auto it, AssetManager::instance()->converters()) {
        QString path(it->templatePath());
        if(!path.isEmpty()) {
            paths.push_back(path);
        }
    }
    paths.removeDuplicates();

    for(auto &it : paths) {
        QFileInfo info(it);
        QString name = fromCamelCase(info.baseName().replace('_', ""));
        m_creationMenu.addAction(name)->setData(it);
    }

    createAction(tr("Open"), SLOT(onItemOpen()))->setData(QVariant::fromValue(ui->contentList));
    createAction(showIn, SLOT(showInGraphicalShell()));
    createAction(tr("Duplicate"), SLOT(onItemDuplicate()))->setData(QVariant::fromValue(ui->contentList));
    createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2))->setData(QVariant::fromValue(ui->contentList));
    createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete))->setData(QVariant::fromValue(ui->contentList));
    m_contentMenu.addSeparator();
    createAction(tr("Reimport"), SLOT(onItemReimport()));

    m_contentTreeMenu.addAction(newFolder)->setData(true);
    m_contentTreeMenu.addAction(showIn, this, SLOT(showInGraphicalShell()));
    m_contentTreeMenu.addSeparator();

    m_contentTreeMenu.addAction(tr("Duplicate"), this, SLOT(onItemDuplicate()))->setData(QVariant::fromValue(ui->contentTree));
    m_contentTreeMenu.addAction(tr("Rename"), this, SLOT(onItemRename()))->setData(QVariant::fromValue(ui->contentTree));
    m_contentTreeMenu.addAction(tr("Delete"), this, SLOT(onItemDelete()))->setData(QVariant::fromValue(ui->contentTree));

    connect(&m_creationMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCreationMenuTriggered(QAction*)));
    connect(&m_contentTreeMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCreationMenuTriggered(QAction*)));
}

void ContentBrowser::onCreationMenuTriggered(QAction *action) {
    const QModelIndex origin = m_listProxy->mapToSource(ui->contentList->rootIndex());

    QString path = ProjectSettings::instance()->contentPath() + "/" + ContentTree::instance()->path(origin);
    QDir dir(path);
    switch(action->data().type()) {
        case QVariant::Bool: {
            QString name("NewFolder");
            AssetManager::findFreeName(name, dir.path());

            QModelIndex index = ContentTree::instance()->setNewAsset(dir.path() + QDir::separator() + name, "", true);
            QModelIndex mapped = m_listProxy->mapFromSource(index);
            ui->contentList->setCurrentIndex(mapped);
            ui->contentList->edit(mapped);
        } break;
        case QVariant::String: {
            QFileInfo info(action->data().toString());
            QString name = QString("New") + info.baseName();
            QString suff = QString(".") + info.suffix();
            AssetManager::findFreeName(name, dir.path(), suff);

            QModelIndex index = ContentTree::instance()->setNewAsset(dir.path() + QDir::separator() + name, action->data().toString());
            QModelIndex mapped = m_listProxy->mapFromSource(index);
            ui->contentList->setCurrentIndex(mapped);
            ui->contentList->edit(mapped);
        } break;
        default: break;
    }
}

void ContentBrowser::onFilterMenuTriggered(QAction *) {
    QStringList list;
    foreach(QAction *it, m_filterMenu->findChildren<QAction *>()) {
        if(it->isChecked()) {
            list.append(it->text());
        }
    }
    m_listProxy->setContentTypes(list);
}

void ContentBrowser::onFilterMenuAboutToShow() {
    for(auto &it : AssetManager::instance()->labels()) {
        if(!it.isEmpty()) {
            QAction *child = m_filterMenu->findChild<QAction *>(it);
            if(child == nullptr) {
                QAction *a = new QAction(it, m_filterMenu);
                a->setCheckable(true);
                a->setObjectName(it);
                m_filterMenu->addAction(a);
            }
        }
    }
}

void ContentBrowser::onItemOpen() {
    QAction *action = qobject_cast<QAction*>(sender());
    if(action) {
        QAbstractItemView *view = qvariant_cast<QAbstractItemView*>(action->data());

        on_contentList_doubleClicked(view->currentIndex());
    }
}

void ContentBrowser::onItemRename() {
    QAction *action = qobject_cast<QAction*>(sender());
    if(action) {
        QAbstractItemView *view = qvariant_cast<QAbstractItemView*>(action->data());
        view->edit(view->currentIndex());
    }
}

void ContentBrowser::onItemDuplicate() {
    QAction *action = qobject_cast<QAction*>(sender());
    if(action) {
        QAbstractItemView *view = qvariant_cast<QAbstractItemView*>(action->data());
        QSortFilterProxyModel *filter = static_cast<QSortFilterProxyModel*>(view->model());
        BaseObjectModel *model = static_cast<BaseObjectModel*>(filter->sourceModel());

        QModelIndex index = filter->mapToSource(view->currentIndex());
        QString path = model->path(index);
        AssetManager::instance()->duplicateResource(dynamic_cast<QTreeView*>(view) != nullptr ? QFileInfo(path).fileName() : path);
    }
}

void ContentBrowser::onItemReimport() {
    QModelIndex index = m_listProxy->mapToSource(ui->contentList->currentIndex());
    ContentTree::instance()->reimportResource(index);
}

void ContentBrowser::onItemDelete() {
    QAction *action = qobject_cast<QAction*>(sender());
    if(action) {
        QAbstractItemView *view = qvariant_cast<QAbstractItemView*>(action->data());
        QSortFilterProxyModel *filter = static_cast<QSortFilterProxyModel*>(view->model());
        BaseObjectModel *model = static_cast<BaseObjectModel*>(filter->sourceModel());

        QMessageBox msgBox(QMessageBox::Question, tr("Delete Assets"),
                           tr("This action cannot be reverted. Do you want to delete selected assets?"),
                           QMessageBox::Yes | QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes) {
            foreach(auto &it, view->selectionModel()->selectedIndexes()) {
                QObject *item = static_cast<QObject *>(filter->mapToSource(it).internalPointer());
                if(item) {
                    AssetManager::instance()->removeResource(item->objectName());
                }
            }

            emit model->layoutAboutToBeChanged();
            emit model->layoutChanged();

            view->clearSelection();
        }
    }
}

void ContentBrowser::on_findContent_textChanged(const QString &arg1) {
    m_listProxy->setFilterFixedString(arg1);
}

void ContentBrowser::on_contentTree_clicked(const QModelIndex &index) {
    ui->findContent->clear();
    QModelIndex origin = m_treeProxy->mapToSource(index);
    ui->contentList->setRootIndex(m_listProxy->mapFromSource(origin));
}

void ContentBrowser::on_contentList_doubleClicked(const QModelIndex &index) {
    const QModelIndex origin = m_listProxy->mapToSource(index);

    ContentTree *inst = ContentTree::instance();
    if(inst->isDir(origin)) {
        ui->contentList->setRootIndex(index);
    } else {
        emit openEditor(inst->path(origin));
    }
}

QAction* ContentBrowser::createAction(const QString &name, const char *member, const QKeySequence &shortcut) {
    QAction *a = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), this, member);
    ui->contentList->addAction(a);
    m_contentMenu.addAction(a);
    return a;
}

void ContentBrowser::on_contentList_customContextMenuRequested(const QPoint &pos) {
    QWidget *w = static_cast<QWidget*>(QObject::sender());
    if(ui->contentList->selectionModel()->selectedIndexes().empty()) {
        m_creationMenu.exec(w->mapToGlobal(pos));
    } else {
        m_contentMenu.exec(w->mapToGlobal(pos));
    }
}

void ContentBrowser::on_contentTree_customContextMenuRequested(const QPoint &pos) {
    QWidget* w = static_cast<QWidget*>(QObject::sender());
    if(!ui->contentTree->selectionModel()->selectedIndexes().empty())
    {
        m_contentTreeMenu.exec(w->mapToGlobal(pos));
    }
}

void ContentBrowser::on_contentList_clicked(const QModelIndex &index) {
    const QModelIndex origin = m_listProxy->mapToSource(index);

    QString source = ContentTree::instance()->path(origin);
    QString path(source);
    if(!source.contains(".embedded/")) {
        path = ProjectSettings::instance()->contentPath() + QDir::separator() + source;
    }

    m_settings = AssetManager::instance()->fetchSettings(path);
    if(m_settings) {
        if(m_commitRevert) {
            m_commitRevert->setObject(m_settings);
        } else {

        }
        emit assetsSelected({m_settings});
    }
}

void ContentBrowser::importAsset() {
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select files to import"),
                                                      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                      tr("All (*.*)") );

    const QModelIndex origin = m_listProxy->mapToSource(ui->contentList->rootIndex());

    QString target = ProjectSettings::instance()->contentPath() + "/" + ContentTree::instance()->path(origin);

    foreach(auto &it, files) {
        AssetManager::instance()->import(it, target);
    }
}

void ContentBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void ContentBrowser::showInGraphicalShell() {
    QString path;
    QModelIndexList list = ui->contentList->selectionModel()->selectedIndexes();
    if(list.empty()) {
        path = ContentTree::instance()->path(m_listProxy->mapToSource(ui->contentList->rootIndex()));
    } else {
        path = ContentTree::instance()->path(m_listProxy->mapToSource(list.first()));
    }

    path = ProjectSettings::instance()->contentPath() + QDir::separator() + path;

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
    const QFileInfo fileInfo(path);
    QStringList scriptArgs;
    scriptArgs << fileInfo.absoluteFilePath();
    QProcess::execute(QLatin1String("xdg-open"), scriptArgs);
#endif
}
