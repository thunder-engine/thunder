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
#include <QDrag>

#include <global.h>

#include "contenttree.h"

#include "assetmanager.h"
#include <editor/projectmanager.h>

#include "../propertyedit/propertymodel.h"

#define ICON_SIZE 128

namespace {
    const char *gTemplateName("${templateName}");
}

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

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        return QStyledItemDelegate::sizeHint(option, index);
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
        int32_t width = editor->width();
        QStyledItemDelegate::setEditorData(editor, index);
        editor->setFixedWidth(width);
    }

    float m_Scale;
};

ContentBrowser::ContentBrowser(QWidget* parent) :
        QWidget(parent),
        ui(new Ui::ContentBrowser) {

    ui->setupUi(this);

    ContentItemDeligate *treeDeligate = new ContentItemDeligate;
    treeDeligate->setItemScale(20.0f / ICON_SIZE);

    m_pTreeProxy = new ContentTreeFilter(this);
    m_pTreeProxy->setSourceModel(ContentTree::instance());
    m_pTreeProxy->setContentTypes({0});
    m_pTreeProxy->sort(0);

    ui->contentTree->setItemDelegate(treeDeligate);
    ui->contentTree->setModel(m_pTreeProxy);
    ui->contentTree->expandToDepth(1);

    // Content list
    m_pContentDeligate = new ContentItemDeligate;
    m_pContentDeligate->setItemScale(0.75f);

    m_pListProxy = new ContentTreeFilter(this);
    m_pListProxy->setSourceModel(ContentTree::instance());
    m_pListProxy->sort(0);

    ui->contentList->setItemDelegate(m_pContentDeligate);
    ui->contentList->setModel(m_pListProxy);

    ui->contentList->setRootIndex(m_pListProxy->mapFromSource(ContentTree::instance()->getContent()));

    connect(ContentTree::instance(), &ContentTree::layoutChanged, m_pTreeProxy, &ContentTreeFilter::invalidate);

    m_pFilterMenu = new QMenu(this);

    ui->filterButton->setMenu(m_pFilterMenu);
    connect(m_pFilterMenu, &QMenu::triggered, this, &ContentBrowser::onFilterMenuTriggered);
    connect(m_pFilterMenu, &QMenu::aboutToShow, this, &ContentBrowser::onFilterMenuAboutToShow);

    readSettings();
    createContextMenus();
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

void ContentBrowser::createContextMenus() {
    QString showIn(tr("Show in Explorer"));
    QLabel *label = new QLabel(tr("Create Asset"), this);
    QWidgetAction *a = new QWidgetAction(&m_CreationMenu);
    a->setDefaultWidget(label);

    m_CreationMenu.addAction(tr("New Folder"))->setData(true);
    m_CreationMenu.addAction(showIn, this, SLOT(showInGraphicalShell()));
    m_CreationMenu.addSeparator();
    m_CreationMenu.addAction(a);

    QStringList paths;
    foreach(auto it, AssetManager::instance()->converters()) {
        QString path = it->templatePath();
        if(!path.isEmpty()) {
            paths.push_back(path);
        }
    }
    paths.removeDuplicates();

    for(auto &it : paths) {
        QFileInfo info(it);
        QString name = fromCamelCase(info.baseName().replace('_', ""));
        m_CreationMenu.addAction(name)->setData(it);
    }

    createAction(tr("Open"), SLOT(onItemOpen()))->setData(QVariant::fromValue(ui->contentList));
    createAction(showIn, SLOT(showInGraphicalShell()));
    createAction(tr("Duplicate"), SLOT(onItemDuplicate()))->setData(QVariant::fromValue(ui->contentList));
    createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2))->setData(QVariant::fromValue(ui->contentList));
    createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete))->setData(QVariant::fromValue(ui->contentList));
    m_ContentMenu.addSeparator();
    createAction(tr("Reimport"), SLOT(onItemReimport()));

    m_contentTreeMenu.addAction(tr("New Folder"))->setData(true);
    m_contentTreeMenu.addAction(showIn, this, SLOT(showInGraphicalShell()));
    m_contentTreeMenu.addSeparator();

    m_contentTreeMenu.addAction(tr("Duplicate"), this, SLOT(onItemDuplicate()))->setData(QVariant::fromValue(ui->contentTree));
    m_contentTreeMenu.addAction(tr("Rename"), this, SLOT(onItemRename()))->setData(QVariant::fromValue(ui->contentTree));
    m_contentTreeMenu.addAction(tr("Delete"), this, SLOT(onItemDelete()))->setData(QVariant::fromValue(ui->contentTree));

    connect(&m_CreationMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCreationMenuTriggered(QAction*)));
    connect(&m_contentTreeMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCreationMenuTriggered(QAction*)));
}

void ContentBrowser::onCreationMenuTriggered(QAction *action) {
    const QModelIndex origin = m_pListProxy->mapToSource(ui->contentList->rootIndex());

    QString path = ProjectManager::instance()->contentPath() + "/" + ContentTree::instance()->path(origin);
    QDir dir(path);
    switch(action->data().type()) {
        case QVariant::Bool: {
            QString name("NewFolder");
            AssetManager::findFreeName(name, dir.path());
            dir.mkdir(name);
        } break;
        case QVariant::String: {
            QFileInfo info(action->data().toString());
            QString name = QString("New") + info.baseName();
            QString suff = QString(".") + info.suffix();
            AssetManager::findFreeName(name, dir.path(), suff);

            QFile file(action->data().toString());
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
    QStringList list;
    foreach(QAction *it, m_pFilterMenu->findChildren<QAction *>()) {
        if(it->isChecked()) {
            list.append(it->text());
        }
    }
    m_pListProxy->setContentTypes(list);
}

void ContentBrowser::onFilterMenuAboutToShow() {
    for(auto &it : AssetManager::instance()->labels()) {
        if(!it.isEmpty()) {
            QAction *child = m_pFilterMenu->findChild<QAction *>(it);
            if(child == nullptr) {
                QAction *a = new QAction(it, m_pFilterMenu);
                a->setCheckable(true);
                a->setObjectName(it);
                m_pFilterMenu->addAction(a);
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
        QFileInfo info = dynamic_cast<QTreeView*>(view) != nullptr ? QFileInfo(path).fileName() : QFileInfo(path);
        AssetManager::instance()->duplicateResource(info);
    }
}

void ContentBrowser::onItemReimport() {
    QModelIndex index = m_pListProxy->mapToSource(ui->contentList->currentIndex());
    ContentTree::instance()->reimportResource(index);
}

void ContentBrowser::onItemDelete() {
    QAction *action = qobject_cast<QAction*>(sender());
    if(action) {
        QAbstractItemView *view = qvariant_cast<QAbstractItemView*>(action->data());
        QSortFilterProxyModel *filter = static_cast<QSortFilterProxyModel*>(view->model());
        BaseObjectModel *model = static_cast<BaseObjectModel*>(filter->sourceModel());

        QMessageBox msgBox(QMessageBox::Question, tr("Delete Asset"),
                           tr("This action cannot be reverted. Do you want to delete selected asset?"),
                           QMessageBox::Yes | QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes) {
            model->removeResource(filter->mapToSource(view->currentIndex()));
        }
    }
}

void ContentBrowser::assetUpdated() {
    ui->contentList->update();
}

void ContentBrowser::on_findContent_textChanged(const QString &arg1) {
    m_pListProxy->setFilterFixedString(arg1);
}

void ContentBrowser::on_contentTree_clicked(const QModelIndex &index) {
    ui->findContent->clear();
    QModelIndex origin = m_pTreeProxy->mapToSource(index);
    ui->contentList->setRootIndex(m_pListProxy->mapFromSource(origin));
}

void ContentBrowser::on_contentList_doubleClicked(const QModelIndex &index) {
    const QModelIndex origin = m_pListProxy->mapToSource(index);

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
    m_ContentMenu.addAction(a);
    return a;
}

void ContentBrowser::on_contentList_customContextMenuRequested(const QPoint &pos) {
    QWidget *w = static_cast<QWidget*>(QObject::sender());
    if(ui->contentList->selectionModel()->selectedIndexes().empty()) {
        m_CreationMenu.exec(w->mapToGlobal(pos));
    } else {
        m_ContentMenu.exec(w->mapToGlobal(pos));
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
    const QModelIndex origin = m_pListProxy->mapToSource(index);

    QString source = ContentTree::instance()->path(origin);
    QFileInfo path(source);
    if(!source.contains(".embedded/")) {
        path = ProjectManager::instance()->contentPath() + QDir::separator() + source;
    }

    emit assetSelected(AssetManager::instance()->fetchSettings(path));
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
        path = ContentTree::instance()->path(m_pListProxy->mapToSource(ui->contentList->rootIndex()));
    } else {
        path = ContentTree::instance()->path(m_pListProxy->mapToSource(list.first()));
    }

    path = ProjectManager::instance()->contentPath() + QDir::separator() + path;

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
