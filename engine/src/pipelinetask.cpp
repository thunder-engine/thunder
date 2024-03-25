#include "pipelinetask.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

/*!
    \class PipelineTask
    \brief A base class for all render tasks.
    \inmodule Engine

    All render tasks must be inherited from this class.
*/

PipelineTask::PipelineTask() :
        m_width(1),
        m_height(1),
        m_enabled(true) {

}

PipelineTask::~PipelineTask() {

}
/*!
    This method can be used to analyze a scene graphs for the provided \a world.
*/
void PipelineTask::analyze(World *world) {

}
/*!
    The task will be executed for the provided \a context.
*/
void PipelineTask::exec(PipelineContext &context) {
    A_UNUSED(context);
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
string PipelineTask::inputName(int index) const {
    if(index < m_inputs.size()) {
        return m_inputs[index];
    }
    return string();
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
string PipelineTask::outputName(int index) const {
    if(index < m_outputs.size()) {
        return m_outputs[index].first;
    }
    return string();
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
    Changes a property \a value with given \a name.
*/
void PipelineTask::setProperty(const string &name, const Variant &value) {
    A_UNUSED(name);
    A_UNUSED(value);
}
/*!
    A callback to react on chage of \a settings.
*/
void PipelineTask::setSettings(const PostProcessSettings &settings) {
    A_UNUSED(settings);
}
/*!
    Sets task to \a enable or disable.
    The disabled effect will not be executed.
*/
void PipelineTask::setEnabled(bool enable) {
    m_enabled = enable;
}
/*!
    Returns true if task is enabled; otherwise returns false.
*/
bool PipelineTask::isEnabled() const {
    return m_enabled;
}
