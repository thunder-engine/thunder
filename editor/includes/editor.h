#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
#ifdef EDITOR_LIBRARY
#define EDITOR_EXPORT __declspec(dllexport)
#else
#define EDITOR_EXPORT __declspec(dllimport)
#endif
#else
#define EDITOR_EXPORT
#endif

#endif // EDITOR_H
