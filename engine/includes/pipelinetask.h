#ifndef PIPELINETASK_H
#define PIPELINETASK_H

#include <stdint.h>

#include <pipelinecontext.h>
#include <engine.h>

class Texture;

class ENGINE_EXPORT PipelineTask : public Object {
    A_OBJECT(PipelineTask, Object, System)

public:
    PipelineTask();
    ~PipelineTask();

    virtual void setContext(PipelineContext *context);

    virtual void analyze(World *world);
    virtual void exec();

    virtual void resize(int width, int height);

    int inputCount() const;
    TString inputName(int index) const;
    virtual void setInput(int index, Texture *source);

    int outputCount() const;
    TString outputName(int index) const;
    virtual Texture *output(int index);

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    struct Group {
        MaterialInstance *instance = nullptr;

        Mesh *mesh = nullptr;

        uint32_t subMesh = 0;

        uint32_t hash = 0;

        ByteArray buffer;
    };
    typedef std::list<Group> GroupList;

    void filterByLayer(const RenderList &in, GroupList &out, int layer) const;

    void group(const GroupList &in, GroupList &out) const;

protected:
    std::vector<TString> m_inputs;
    std::vector<std::pair<TString, Texture *>> m_outputs;

    PipelineContext *m_context;

    int m_width;
    int m_height;

    bool m_enabled;

};

#endif // PIPELINETASK_H
