#ifndef ANGELCORE_H
#define ANGELCORE_H

class asIScriptEngine;

void registerCore(asIScriptEngine *engine, bool generic);

void registerEngine(asIScriptEngine *engine, bool generic);

void registerMath(asIScriptEngine *engine, bool generic);

void registerInput(asIScriptEngine *engine, bool generic);

void registerTimer(asIScriptEngine *engine, bool generic);

#endif // ANGELCORE_H
