#include "plugindelegate.h"

PluginDelegate::PluginDelegate(QObject* parent) :
        QStyledItemDelegate(parent) {

}

PluginDelegate::~PluginDelegate() {

}

void PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter, opt, index);
}
