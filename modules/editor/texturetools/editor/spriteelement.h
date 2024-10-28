#ifndef SPRITEELEMENT_H
#define SPRITEELEMENT_H

#include <QObject>

#include <amath.h>

#include "../converter/textureconverter.h"

class SpriteController;

class SpriteElement : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName DESIGNABLE true USER true);
    Q_PROPERTY(Vector2 position READ position WRITE setPosition DESIGNABLE true USER true);
    Q_PROPERTY(Vector2 size READ size WRITE setSize DESIGNABLE true USER true);
    Q_PROPERTY(Vector4 borderTRBL READ border WRITE setBorder DESIGNABLE true USER true);
    Q_PROPERTY(Vector2 pivot READ pivot WRITE setPivot DESIGNABLE true USER true);

public:
    SpriteElement();

    void setController(SpriteController *controller);
    void setSettings(TextureImportSettings *settings);

    std::string key() const { return m_key; }
    void setKey(const std::string &key);

    QString name() const;
    void setName(const QString &name);

    Vector2 position() const;
    void setPosition(const Vector2 &position);

    Vector2 size() const;
    void setSize(const Vector2 &position);

    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);

    Vector4 border() const;
    void setBorder(const Vector4 &border);

private:
    void updateController(const TextureImportSettings::Element &element);

private:
    std::string m_key;

    SpriteController *m_controller;

    TextureImportSettings *m_settings;

};

#endif // SPRITEELEMENT_H
