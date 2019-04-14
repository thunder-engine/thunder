#ifndef ANGELCORE_H
#define ANGELCORE_H

class asIScriptEngine;

void registerCore(asIScriptEngine *engine);

void registerMath(asIScriptEngine *engine);

void registerInput(asIScriptEngine *engine);

void registerTimer(asIScriptEngine *engine);

#endif // ANGELCORE_H
