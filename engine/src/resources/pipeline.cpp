#include "resources/pipeline.h"

namespace {
    const char *gTasks = "Tasks";
    const char *gLinks = "Links";
}

/*!
    \class Pipeline
    \brief The Pipeline class represents a pipeline structure consisting of render tasks and their relations.
    \inmodule Resources

    The Pipeline class allows users to manage render tasks and their connections within a pipeline.
    Users can query information about render tasks, their names, and links between them.
    The class also provides methods for loading and saving user-specific data related to the pipeline.
*/

/*!
    Returns the number of render tasks in the pipeline.
*/
int Pipeline::renderTasksCount() const {
    return m_renderTasks.size();
}
/*!
    Returns the name of the render task at the specified \a index.
*/
String Pipeline::renderTaskName(int index) const {
    if(index < m_renderTasks.size()) {
        return m_renderTasks[index];
    }
    return std::string();
}
/*!
    Returns the number of links between render tasks in the pipeline.
*/
int Pipeline::renderTasksLinksCount() const {
    return m_renderTasksLinks.size();
}
/*!
    Returns the link information for the render task at the specified \a index.
*/
Pipeline::Link Pipeline::renderTaskLink(int index) const {
    if(index < m_renderTasksLinks.size()) {
        return m_renderTasksLinks[index];
    }
    return Pipeline::Link();
}
/*!
    \internal
*/
void Pipeline::loadUserData(const VariantMap &data) {
    {
        m_renderTasks.clear();
        auto it = data.find(gTasks);
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
        auto it = data.find(gLinks);
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
/*!
    \internal
*/
VariantMap Pipeline::saveUserData() const {
    VariantMap result;

    VariantList tasks;
    for(auto &it : m_renderTasks) {
        tasks.push_back(it);
    }
    result[gTasks] = tasks;

    VariantList links;
    for(auto &it : m_renderTasksLinks) {
        VariantList fields;
        fields.push_back(it.source);
        fields.push_back(it.target);
        fields.push_back(it.output);
        fields.push_back(it.input);

        links.push_back(fields);
    }
    result[gLinks] = links;

    return result;
}
