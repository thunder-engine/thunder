#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

const QString gContent("content");
const QString gCache("cache");
const QString gImport("assets");
const QString gThumbnails("thumbnails");
const QString gGenerated("generated");
const QString gPlugins("plugins");
const QString gType("type");
const QString gIcon("icon");
const QString gSubItems("subitems");
const QString gSettings("settings");
const QString gPlatforms("platforms");
const QString gModules("modules");

const QString gProjectExt(".forge");
const QString gMetaExt(".set");
const QString gIndex("index");

const QString gRhi("rhi");

#if defined(Q_OS_WIN)
const QString gShared(".dll");
const QString gStatic(".lib");
const QString gPrefix("");
const QString gApplication(".exe");
#elif defined(Q_OS_MAC)
const QString gShared(".dylib");
const QString gStatic(".a");
const QString gPrefix("lib");
const QString gApplication(".app");
#elif defined(Q_OS_UNIX)
const QString gShared(".so");
const QString gStatic(".a");
const QString gPrefix("lib");
const QString gApplication("");
#endif
const QString gMimeContent("text/content");
const QString gMimeObject("text/object");
const QString gMimeComponent("text/component");

#endif // CONFIG_H

