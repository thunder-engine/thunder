#include "scheme/schemeview.h"

#include <QMenu>

#include "scheme/abstractschememodel.h"

#include <QQmlContext>
#include <QQuickItem>

SchemeView::SchemeView(QWidget *parent) :
        QQuickWidget(parent),
        m_createMenu(new QMenu(this)),
        m_model(nullptr),
        m_selectedItem(nullptr),
        m_node(-1),
        m_port(-1),
        m_out(false) {

    setContextMenuPolicy(Qt::CustomContextMenu);
    setResizeMode(QQuickWidget::SizeRootObjectToView);

}

void SchemeView::setModel(AbstractSchemeModel *model, bool state) {
    m_model = model;

    rootContext()->setContextProperty("schemeModel", m_model);
    rootContext()->setContextProperty("stateMachine", QVariant(state));
    setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    QQuickItem *item = rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));
    connect(item, SIGNAL(showContextMenu(int,int,bool)), this, SLOT(onShowContextMenu(int,int,bool)));

    for(auto &it : m_model->nodeList()) {
        QMenu *menu = m_createMenu;
        QStringList list = it.split("/", QString::SkipEmptyParts);

        for(int i = 0; i < list.size(); i++) {
            QString part = list.at(i);
            QAction *action = nullptr;
            for(QAction *act : menu->actions()) {
                if(part == act->objectName()) {
                    action = act;
                    menu = act->menu();
                    break;
                }
            }
            if(action == nullptr) {
                action = menu->addAction(part);
                action->setObjectName(qPrintable(part));
                if(i < (list.size() - 1)) {
                    menu = new QMenu;
                    action->setMenu(menu);
                } else {
                    connect(action, &QAction::triggered, this, &SchemeView::onComponentSelected);
                }
            }
        }
    }
}

void SchemeView::reselect() {
    emit itemSelected(m_selectedItem);
}

void SchemeView::onComponentSelected() {
    m_createMenu->hide();

    QAction *action = static_cast<QAction *>(sender());

    QQuickItem *scheme = rootObject()->findChild<QQuickItem *>("Scheme");
    if(scheme) {
        int x = scheme->property("x").toInt();
        int y = scheme->property("y").toInt();
        float scale = scheme->property("scale").toFloat();

        QQuickItem *canvas = rootObject()->findChild<QQuickItem *>("Canvas");
        if(canvas) {
            int mouseX = canvas->property("mouseX").toInt();
            int mouseY = canvas->property("mouseY").toInt();
            x = (float)(mouseX - x) * scale;
            y = (float)(mouseY - y) * scale;

            if(m_node > -1 && m_port > -1) {
                m_model->createAndLink(action->objectName(), x, y, m_node, m_port, m_out);
            } else {
                m_model->createNode(action->objectName(), x, y);
            }
        }
    }
}

void SchemeView::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_model->node(list.front().toInt());
        if(node) {
            m_selectedItem = static_cast<QObject *>(node->ptr);
            emit itemSelected(m_selectedItem);
        }
    }
}

void SchemeView::onShowContextMenu(int node, int port, bool out) {
    m_node = node;
    m_port = port;
    m_out = out;
    m_createMenu->exec(QCursor::pos());
}
