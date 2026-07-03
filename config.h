#ifndef CONFIG_H
#define CONFIG_H

#include <global.h>

static const char *gContent("content");
static const char *gImport("assets");
static const char *gType("type");
static const char *gIcon("icon");
static const char *gSubItems("subitems");
static const char *gSettings("settings");
static const char *gPlatforms("platforms");

static const char *gProjectExt(".forge");
static const char *gMetaExt("set");
static const char *gIndex("index");

static const char *gRhi("rhi");

#if defined(PLATFORM_WINDOWS)
static const char *gShared(".dll");
static const char *gPrefix("");
static const char *gApplication(".exe");
#elif defined(PLATFORM_MAC)
static const char *gShared(".dylib");
static const char *gPrefix("lib");
static const char *gApplication(".app");
#elif defined(PLATFORM_LINUX)
static const char *gShared(".so");
static const char *gPrefix("lib");
static const char *gApplication("");
#endif
static const char *gMimeContent("text/content");
static const char *gMimeObject("text/object");

#endif // CONFIG_H

