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
#ifndef LEGACYDOCUMENTMODEL_H
#define LEGACYDOCUMENTMODEL_H

#include <QObject>

#include <editor.h>

class EDITOR_EXPORT LegacyDocumentModel : public QObject {
    Q_OBJECT

public:
    LegacyDocumentModel();
    ~LegacyDocumentModel();

    void addEditor(LegacyAssetEditor *editor);

    LegacyAssetEditor *openFile(const TString &path);

    std::list<LegacyAssetEditor *> documents();

signals:
    void updated();

    void selectionChanaged();

public slots:
    void closeFile(LegacyAssetEditor *editor);

private:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onLoadAsset(QString path);

protected:
    std::list<LegacyAssetEditor *> m_documents;

    std::map<TString, LegacyAssetEditor *> m_editors;

};

#endif // LEGACYDOCUMENTMODEL_H
