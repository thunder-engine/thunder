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

#include "module.h"

/*!
    \class Module
    \brief A base interface for all engine modules.
    \inmodule Engine

    Thunder Engine uses modules to extend engine capabilities.
    Every engine module must be inherited from the Module class.
    This class contains basic information about the content of the module and creates some of the necessary in-module systems.
*/

/*!
    Returns a module manifest in JSON format.
*/
const char *Module::metaInfo() const {
    return nullptr;
}

/*!
    This function is a facory for the module. It return a pointer to constructed object with given type \a name.
*/
void *Module::getObject(const char *name) {
    A_UNUSED(name);
    return nullptr;
}
