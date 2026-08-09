#include "Sprite.hpp"

bool Sprite::showDebug = false;

Sprite::Sprite(Texture2D* texture){
    this->texture = texture;
    reflectHorizontally = false;
}

void Sprite::draw() const{
    if(!texture) return;

    float width  = texture->width  * scale;
    float height = texture->height * scale;

    Rectangle source = {
        0, 0, (float)texture->width, -(float)texture->height
    };

    Rectangle destination = {
        position.x,
        position.y,
        width, height
    };

    if (reflectHorizontally){
        source.width *= -1;
    }

    Vector2 origin = {
        width * pivot.x,
        height * pivot.y
    };

    DrawTexturePro(*texture, source, destination, origin, rotation, WHITE);

    if(showDebug)
        DrawCircle((int)position.x, (int)position.y, 2.0f, LIME);
}

Vector2 Sprite::getPosition() const{
    return position;
}

void Sprite::setPosition(Vector2 position){
    this->position.x = position.x;
    this->position.y = position.y;
}

float Sprite::getRotation() const{
    return rotation;
}

void Sprite::setRotation(float rotation){
    this->rotation = rotation;
}

float Sprite::getScale() const{
    return scale;
}

void Sprite::setScale(float scale){
    this->scale = scale;
}

bool Sprite::isReflectedHorizopntally() const{
    return reflectHorizontally;
}
void Sprite::ReflectHorizontally(bool state){
    reflectHorizontally = state;
}

Vector2 Sprite::getPivot() const{
    return pivot;
}

void Sprite::setPivot(Vector2 pivot){
    this->pivot.x = pivot.x;
    this->pivot.y = pivot.y;
}