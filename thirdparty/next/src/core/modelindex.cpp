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

#include "core/modelindex.h"

#include "core/abstractitemmodel.h"

ModelIndex::ModelIndex() :
    m_model(nullptr),
    m_row(-1),
    m_column(0),
    m_uuid(0) {
}

bool ModelIndex::isValid() const {
    return m_model != nullptr && m_row >= 0;
}

int ModelIndex::row() const {
    return m_row;
}

int ModelIndex::column() const {
    return m_column;
}

const AbstractItemModel *ModelIndex::model() const {
    return m_model;
}

ModelIndex ModelIndex::parent() const {
    return m_model ? m_model->parent(*this) : ModelIndex();
}

uint32_t ModelIndex::internalId() const {
    return m_uuid;
}

bool ModelIndex::operator==(const ModelIndex &other) const {
    return m_model == other.m_model && m_row == other.m_row && m_column == other.m_column && m_uuid == other.m_uuid;
}

bool ModelIndex::operator!=(const ModelIndex &other) const {
    return !(*this == other);
}
