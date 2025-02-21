/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "contextswitch_p.h"
#include "definition.h"
#include "definition_p.h"
#include "repository.h"

#include <QDebug>

using namespace KSyntaxHighlighting;

bool ContextSwitch::isStay() const
{
    return m_popCount == 0 && !m_context && m_contextName.isEmpty() && m_defName.isEmpty();
}

int ContextSwitch::popCount() const
{
    return m_popCount;
}

Context *ContextSwitch::context() const
{
    return m_context;
}

void ContextSwitch::parse(const QString &contextInstr)
{
    if (contextInstr.isEmpty() || contextInstr == QLatin1String("#stay"))
        return;

    if (contextInstr.startsWith(QLatin1String("#pop!"))) {
        ++m_popCount;
        m_contextName = contextInstr.mid(5);
        return;
    }

    if (contextInstr.startsWith(QLatin1String("#pop"))) {
        ++m_popCount;
        parse(contextInstr.mid(4));
        return;
    }

    const auto idx = contextInstr.indexOf(QLatin1String("##"));
    if (idx >= 0) {
        m_contextName = contextInstr.left(idx);
        m_defName = contextInstr.mid(idx + 2);
    } else {
        m_contextName = contextInstr;
    }
}

void ContextSwitch::resolve(const Definition &def)
{
    auto d = def;
    if (!m_defName.isEmpty()) {
        d = DefinitionData::get(def)->repo->definitionForName(m_defName);
        auto data = DefinitionData::get(d);
        data->load();
        if (m_contextName.isEmpty())
            m_context = data->initialContext();
    }

    if (!m_contextName.isEmpty()) {
        m_context = DefinitionData::get(d)->contextByName(m_contextName);
        if (!m_context)
            qWarning() << "cannot find context" << m_contextName << "in" << def.name();
    }
}
