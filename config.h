#ifndef COMMON_H
#define COMMON_H

#include <QDir>
#include <QFontMetrics>

const QString gContent("content");
const QString gCache("cache");
const QString gImport("import");
const QString gIcons("thumbnails");
const QString gGenerated("generated");
const QString gPlugins("plugins");
const QString gType("type");
const QString gSettings("settings");

const QString gProjectExt(".forge");
const QString gMetaExt(".set");
const QString gIndex("index");

#if _WIN32
const QString gShared(".dll");
const QString gStatic(".lib");
const QString gApplication(".exe");
#elif __APPLE__
const QString gShared(".dylib");
const QString gStatic(".a");
const QString gApplication(".app");
#elif __linux__
const QString gShared(".so");
const QString gStatic(".a");
const QString gApplication("");
#endif
const QString gMimeContent("text/content");

const QString gDefaultFont("Helvetica Neue");

const int gFontSize     = 12;
const int gRoundness    = 8;

const QFont gFont(gDefaultFont, gFontSize);
const QFontMetrics gMetrics(gFont);

#endif // COMMON_H

