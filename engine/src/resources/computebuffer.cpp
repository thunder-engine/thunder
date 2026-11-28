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

#include "resources/computebuffer.h"

/*!
    \class ComputeBuffer
    \brief The ComputeBuffer class represents a buffer of data used in compute shaders.
    \inmodule Resources

    The ComputeBuffer class provides functionality to manage and manipulate data buffers used in compute shaders.
    The count method retrieves the number of elements, stride gets and sets the stride of the buffer, and data gets the current data stored in the buffer.
    The setData method allows updating the buffer data and marks the buffer as dirty.
*/

ComputeBuffer::ComputeBuffer() :
        m_bufferDirty(false) {

}

ComputeBuffer::~ComputeBuffer() {

}
/*!
    Gets the number of elements in the compute buffer.
*/
uint32_t ComputeBuffer::count() const {
    return 0;
}
/*!
    Returns stride, representing the size of a single element in the buffer.
*/
uint32_t ComputeBuffer::stride() const {
    return 0;
}
/*!
    Sets the \a stride of the compute buffer.
*/
void ComputeBuffer::setStride(uint32_t stride) {
    A_UNUSED(stride);
}
/*!
    Gets the data stored in the compute buffer.
*/
ByteArray ComputeBuffer::data() const {
    return m_buffer;
}
/*!
    Sets the \a data of the compute buffer.

    \note This method updates the buffer data and marks it as dirty.
*/
void ComputeBuffer::setData(const ByteArray &data) {
    m_buffer = data;
    m_bufferDirty = true;
}
