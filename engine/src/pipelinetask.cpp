#include "pipelinetask.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "components/renderable.h"

#include "resources/mesh.h"

/*!
    \class PipelineTask
    \brief A base class for all render tasks.
    \inmodule Engine

    All render tasks must be inherited from this class.
*/

PipelineTask::PipelineTask() :
        m_context(nullptr),
        m_width(1),
        m_height(1),
        m_enabled(true) {

}

PipelineTask::~PipelineTask() {

}
/*!
    Sets the pipeline \a context which the given task belongs.
*/
void PipelineTask::setContext(PipelineContext *context) {
    m_context = context;
}
/*!
    This method can be used to analyze a scene graphs for the provided \a world.
*/
void PipelineTask::analyze(World *world) {

}
/*!
    The task will be executed for the provided \a context.
*/
void PipelineTask::exec() {

}
/*!
    A callback to react on screen \a width and \a height changed.
*/
void PipelineTask::resize(int width, int height) {
    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        for(auto &it : m_outputs) {
            if(it.second) {
                it.second->resize(m_width, m_height);
            }
        }
    }
}
/*!
    Return the number of inputs.
*/
int PipelineTask::inputCount() const {
    return m_inputs.size();
}
/*!
    Returns by \a index a name of input.
*/
TString PipelineTask::inputName(int index) const {
    if(index < m_inputs.size()) {
        return m_inputs[index];
    }
    return TString();
}
/*!
    Set a \a source texture with given \a index to use it in the render task.
*/
void PipelineTask::setInput(int index, Texture *source) {
    A_UNUSED(index);
    A_UNUSED(source);
}
/*!
    Return the number of outputs.
*/
int PipelineTask::outputCount() const {
    return m_outputs.size();
}
/*!
    Returns by \a index a name of output.
*/
TString PipelineTask::outputName(int index) const {
    if(index < m_outputs.size()) {
        return m_outputs[index].first;
    }
    return TString();
}
/*!
    Returns by \a index a result of task as a render texture.
*/
Texture *PipelineTask::output(int index) {
    if(index < m_outputs.size()) {
        return m_outputs[index].second;
    }
    return nullptr;
}
/*!
    Sets task to \a enable or disable.
    The disabled effect will not be executed.
*/
void PipelineTask::setEnabled(bool enable) {
    m_enabled = enable;

    if(m_context) {
        m_context->invalidateTasks();
    }
}
/*!
    Returns true if task is enabled; otherwise returns false.
*/
bool PipelineTask::isEnabled() const {
    return m_enabled;
}
/*!
    Filters \a out an \a in renderable components by it's material \a layer.
*/
void PipelineTask::filterByLayer(const RenderList &in, GroupList &out, int layer) const {
    for(auto it : in) {
        for(int i = 0; i < it->materialsCount(); i++) {
            MaterialInstance *instance = it->materialInstance(i);
            if(instance && instance->material()->layers() & layer) {
                Mesh *mesh = it->meshToDraw(i);
                if(mesh) {
                    uint32_t hash = instance->hash();
                    Mathf::hashCombine(hash, mesh->uuid());

                    out.push_back({instance, mesh, it->subMesh(i), hash});
                }
            }
        }
    }

    out.sort([](const Group &left, const Group &right) {
        int p1 = left.instance->priority();
        int p2 = right.instance->priority();
        if(p1 == p2) {
            return left.hash < right.hash;
        }
        return p1 < p2;
    });
}
/*!
    Groups elements from \a in list into \a out rendering instances.
*/
void PipelineTask::group(const GroupList &in, GroupList &out) const {
    Group last;

    for(auto &it : in) {
        if(last.hash != it.hash || (last.instance != nullptr && last.instance->material() != it.instance->material())) {
            if(last.instance != nullptr) {
                out.push_back(last);
            }

            last = it;
            auto &buffer = it.instance->rawUniformBuffer();
            last.buffer.insert(last.buffer.begin(), buffer.begin(), buffer.begin() + it.instance->instanceSize());
        } else {
            auto &buffer = it.instance->rawUniformBuffer();
            last.buffer.insert(last.buffer.end(), buffer.begin(), buffer.begin() + it.instance->instanceSize());
        }
    }

    // do the last insert
    if(last.instance != nullptr) {
        out.push_back(last);
    }
}
