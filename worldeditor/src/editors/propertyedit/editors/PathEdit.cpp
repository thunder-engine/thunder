#include <QApplication>
#include <QFileDialog>

#include <QDebug>

#include "PathEdit.h"

PathEdit::PathEdit(QWidget *parent) :
        QLineEdit(parent) {

    mToolBtn    = new QToolButton(this);
    mToolBtn->setText("...");
    mToolBtn->setCursor(Qt::PointingHandCursor);

    connect(mToolBtn, SIGNAL(clicked()), this, SIGNAL(openFileDlg()));
}

void PathEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);

    QRect wnd       = geometry();
    mToolBtn->setGeometry(QRect(wnd.width()-20, 0, 20, wnd.height() ));
}
