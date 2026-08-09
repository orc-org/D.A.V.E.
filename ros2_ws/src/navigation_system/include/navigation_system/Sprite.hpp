#pragma once
#include <raylib.h>

class Sprite
{
public:
    Sprite(Texture2D* texture);

    void draw() const;

    Vector2 getPosition() const;
    void setPosition(Vector2 position);

    float getRotation() const;
    void setRotation(float rotation);

    float getScale() const;
    void setScale(float scale);

    bool isReflectedHorizopntally() const;
    void ReflectHorizontally(bool state);

    Vector2 getPivot() const;
    void setPivot(Vector2 pivot);



private:
    const Texture2D* texture = nullptr;
    Vector2 position;
    float rotation = 0.0f;
    float scale = 1.0f;
    bool reflectHorizontally;
    static bool showDebug;
    Vector2 pivot = { 0.5f, 0.5f };

};