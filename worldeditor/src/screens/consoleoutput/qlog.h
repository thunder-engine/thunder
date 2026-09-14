/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef QTLOG_H
#define QTLOG_H

#include <QObject>
#include <QDebug>

#include <log.h>

class QLog : public QObject, public LogHandler {
    Q_OBJECT

public:
    void setRecord(Log::LogTypes type, const char *record) {
        emit postRecord(type, QString(record));
        qDebug() << record;
    }

signals:
    void postRecord(uint8_t type, const QString &str);

};

#endif // QTLOG_H
