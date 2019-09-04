#include <QApplication>

#include "ArrayEdit.h"

ArrayEdit::ArrayEdit(QWidget *parent) :
        QLineEdit(parent) {

    m_pAddBtn = new QToolButton(this);
    m_pAddBtn->setObjectName(QString::fromUtf8("m_pAddBtn"));
    m_pAddBtn->setText(QApplication::translate("CurveEditAdd", "+", 0));

    connect(m_pAddBtn, SIGNAL(clicked()), this, SIGNAL(elementAdded()));

    m_pClearBtn = new QToolButton(this);
    m_pClearBtn->setObjectName(QString::fromUtf8("m_pClearBtn"));
    m_pClearBtn->setText(QApplication::translate("CurveEditClear", "{}", 0));

    connect(m_pClearBtn, SIGNAL(clicked()), this, SIGNAL(allCleared()));

    setReadOnly(true);
}

void ArrayEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);

    QRect wnd = geometry();
    m_pClearBtn->setGeometry(QRect(wnd.width()-40, 0, 20, wnd.height() ));
    m_pAddBtn->setGeometry(QRect(wnd.width()-20, 0, 20, wnd.height() ));
}
