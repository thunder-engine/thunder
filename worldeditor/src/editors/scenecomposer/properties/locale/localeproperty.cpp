#include "localeproperty.h"

#include <QDirIterator>
#include <QLocale>

#include "localeedit.h"

QList<QLocale> LocaleProperty::m_locales = QList<QLocale>();

LocaleProperty::LocaleProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

    if(m_locales.isEmpty()) {
        QDirIterator it(":/Translations", QDirIterator::Subdirectories);
        while(it.hasNext()) {
            QFileInfo info(it.next());
            m_locales << QLocale(info.baseName());
        }
    }
}

QWidget *LocaleProperty::createEditor(QWidget *parent) const {
    LocaleEdit *editor = new LocaleEdit(parent);
    m_editor = editor;
    m_editor->setDisabled(isReadOnly());

    for(auto &it : m_locales) {
        QString name = QLocale(it).nativeLanguageName();
        editor->addItem(name.replace(0, 1, name[0].toUpper()), it);
    }

    connect(editor, &LocaleEdit::currentIndexChanged, this, &LocaleProperty::valueChanged);
    return m_editor;
}

bool LocaleProperty::setEditorData(QWidget *editor, const QVariant &data) {
    LocaleEdit *e = static_cast<LocaleEdit *>(editor);
    if(e) {
        int index = e->findData(data.toLocale());
        if(index == -1) {
            return false;
        }
        e->blockSignals(true);
        e->setCurrentIndex(index);
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant LocaleProperty::editorData(QWidget *editor) {
    LocaleEdit *e = static_cast<LocaleEdit *>(editor);
    if(e) {
        return QVariant::fromValue(QLocale(e->currentData().toString()));
    }
    return Property::editorData(editor);
}

void LocaleProperty::valueChanged() {
    LocaleEdit *e = dynamic_cast<LocaleEdit *>(m_editor);
    if(e) {
        setValue(e->currentData());
    }
}
