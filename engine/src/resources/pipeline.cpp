#include "resources/pipeline.h"

#define TASKS "Tasks"
#define LINKS "Links"

int Pipeline::renderTasksCount() const {
    return m_renderTasks.size();
}

string Pipeline::renderTaskName(int index) const {
    if(index < m_renderTasks.size()) {
        return m_renderTasks[index];
    }
    return string();
}

int Pipeline::renderTasksLinksCount() const {
    return m_renderTasksLinks.size();
}

Pipeline::Link Pipeline::renderTaskLink(int index) const {
    if(index < m_renderTasksLinks.size()) {
        return m_renderTasksLinks[index];
    }
    return Pipeline::Link();
}

void Pipeline::loadUserData(const VariantMap &data) {
    {
        m_renderTasks.clear();
        auto it = data.find(TASKS);
        if(it != data.end()) {
            VariantList list = (*it).second.value<VariantList>();
            m_renderTasks.reserve(list.size());
            for(auto &it : list) {
                m_renderTasks.push_back(it.toString());
            }
        }
    }
    {
        m_renderTasksLinks.clear();
        auto it = data.find(LINKS);
        if(it != data.end()) {
            VariantList list = (*it).second.value<VariantList>();
            m_renderTasksLinks.reserve(list.size());
            for(auto &it : list) {
                VariantList fields = it.toList();
                auto i = fields.begin();

                Link link;
                link.source = i->toString();
                i++;
                link.target = i->toString();
                i++;
                link.output = i->toInt();
                i++;
                link.input = i->toInt();

                m_renderTasksLinks.push_back(link);
            }
        }
    }
}

VariantMap Pipeline::saveUserData() const {
    VariantMap result;

    VariantList tasks;
    for(auto &it : m_renderTasks) {
        tasks.push_back(it);
    }
    result[TASKS] = tasks;

    VariantList links;
    for(auto &it : m_renderTasksLinks) {
        VariantList fields;
        fields.push_back(it.source);
        fields.push_back(it.target);
        fields.push_back(it.output);
        fields.push_back(it.input);

        links.push_back(fields);
    }
    result[LINKS] = links;

    return result;
}
