#ifndef COMMON_H
#define COMMON_H

#include <QDir>
#include <QFontMetrics>

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

const QString gProjectExt(".forge");
const QString gMetaExt(".set");
const QString gIndex("index");

#if defined(Q_OS_WIN)
const QString gShared(".dll");
const QString gStatic(".lib");
const QString gApplication(".exe");
#elif defined(Q_OS_MAC)
const QString gShared(".dylib");
const QString gStatic(".a");
const QString gApplication(".app");
#elif defined(Q_OS_UNIX)
const QString gShared(".so");
const QString gStatic(".a");
const QString gApplication("");
#endif
const QString gMimeContent("text/content");
const QString gMimeObject("text/object");

const QString gDefaultFont("Helvetica Neue");

const int gFontSize     = 12;
const int gRoundness    = 8;

const QFont gFont(gDefaultFont, gFontSize);
const QFontMetrics gMetrics(gFont);

#endif // COMMON_H

