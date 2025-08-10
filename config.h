#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

static const char *gContent("content");
static const char *gCache("cache");
static const char *gImport("assets");
static const char *gThumbnails("thumbnails");
static const char *gGenerated("generated");
static const char *gPlugins("plugins");
static const char *gType("type");
static const char *gIcon("icon");
static const char *gSubItems("subitems");
static const char *gSettings("settings");
static const char *gPlatforms("platforms");
static const char *gModules("modules");

static const char *gProjectExt(".forge");
static const char *gMetaExt("set");
static const char *gIndex("index");

static const char *gRhi("rhi");

#if defined(Q_OS_WIN)
static const char *gShared("dll");
static const char *gPrefix("");
static const char *gApplication("exe");
#elif defined(Q_OS_MAC)
static const char *gShared("dylib");
static const char *gPrefix("lib");
static const char *gApplication("app");
#elif defined(Q_OS_UNIX)
static const char *gShared("so");
static const char *gPrefix("lib");
static const char *gApplication("");
#endif
static const char *gMimeContent("text/content");
static const char *gMimeObject("text/object");
static const char *gMimeComponent("text/component");

#endif // CONFIG_H

