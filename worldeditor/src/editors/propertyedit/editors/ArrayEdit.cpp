#include <QApplication>

#include "ArrayEdit.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        QLineEdit(parent) {

    this->mAddBtn       = new QToolButton(this);
    mAddBtn->setObjectName(QString::fromUtf8("mAddBtn"));
    mAddBtn->setText(QApplication::translate("CurveEditAdd", "+", 0));

    connect(mAddBtn, SIGNAL(clicked()), this, SLOT(addElement()));

    this->mClearBtn     = new QToolButton(this);
    mClearBtn->setObjectName(QString::fromUtf8("mClearBtn"));
    mClearBtn->setText(QApplication::translate("CurveEditClear", "{}", 0));

    connect(mClearBtn, SIGNAL(clicked()), this, SLOT(clearAll()));

    setReadOnly(true);
}

void ArrayEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);

    QRect wnd       = geometry();
    mClearBtn->setGeometry(QRect(wnd.width()-40, 0, 20, wnd.height() ));
    mAddBtn  ->setGeometry(QRect(wnd.width()-20, 0, 20, wnd.height() ));
}

void ArrayEdit::addElement() {
    emit elementAdded();
}

void ArrayEdit::clearAll() {
    emit allCleared();
}
