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
const QString gIndex(".index");

const QString gMimeContent("text/content");

const QString gDefaultFont("Helvetica Neue");

const int gFontSize     = 12;
const int gRoundness    = 8;

const QFont gFont(gDefaultFont, gFontSize);
const QFontMetrics gMetrics(gFont);

#endif // COMMON_H

