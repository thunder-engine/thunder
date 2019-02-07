#include <QApplication>
#include <QFileDialog>

#include <QDebug>

#include "PathEdit.h"

#include "projectmanager.h"

PathEdit::PathEdit(QWidget *parent) :
        QLineEdit(parent) {

    mToolBtn    = new QToolButton(this);
    mToolBtn->setText("...");
    mToolBtn->setCursor(Qt::PointingHandCursor);

    connect(mToolBtn, SIGNAL(clicked()), this, SLOT(onFileDialog()));
}

void PathEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);

    QRect wnd = geometry();
    mToolBtn->setGeometry(QRect(wnd.width() - 20, 0, 20, wnd.height() ));
}

void PathEdit::onFileDialog() {
    QString current = ProjectManager::instance()->contentPath();

    QString path = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(parent()),
                                                tr("Select File"),
                                                current,
                                                tr("All Files (*.*)"));

    if(path.length() > 0) {
        path = QDir(current).relativeFilePath(path);

        emit pathChanged(path);
        setText(path);
    }
}
