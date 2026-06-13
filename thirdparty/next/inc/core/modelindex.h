/*
    This file is part of Thunder Next.

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

#ifndef MODELINDEX_H
#define MODELINDEX_H

#include <global.h>
#include <stdint.h>

class AbstractItemModel;

class NEXT_LIBRARY_EXPORT ModelIndex {
public:
    ModelIndex();

    bool isValid() const;

    int row() const;

    int column() const;

    const AbstractItemModel *model() const;

    ModelIndex parent() const;

    uint32_t internalId() const;

    bool operator==(const ModelIndex &other) const;
    bool operator!=(const ModelIndex &other) const;

private:
    friend class AbstractItemModel;

    const AbstractItemModel *m_model;

    int m_row;

    int m_column;

    uint32_t m_uuid;

};

#endif // MODELINDEX_H
