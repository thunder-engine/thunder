#ifndef PIPELINE_H
#define PIPELINE_H

#include "resource.h"

class PipelineTask;

class ENGINE_EXPORT Pipeline : public Resource {
    A_OBJECT(Pipeline, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    struct Link {
        String source;

        String target;

        int output;

        int input;
    };

public:
    int renderTasksCount() const;

    String renderTaskName(int index) const;

    int renderTasksLinksCount() const;

    Link renderTaskLink(int index) const;

    void loadUserData(const VariantMap &data) override;

protected:
    VariantMap saveUserData() const override;

private:
    std::vector<String> m_renderTasks;

    std::vector<Link> m_renderTasksLinks;

};

#endif // PIPELINE_H
