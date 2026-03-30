#include "Brick.h"

Brick::Brick(float _x, float _y, float _w, float _h, BrickType _type)
    : x(_x), y(_y), width(_w), height(_h), destroyed(false), type(_type) {}

void Brick::Draw() const {
    if (destroyed) return;

    Color color;

    switch (type) {
        case NORMAL: color = GREEN; break;
        case SPLIT: color = YELLOW; break;
        case DOUBLE_SCORE: color = PURPLE; break;
    }

    DrawRectangle((int)x, (int)y, (int)width, (int)height, color);
}

Rectangle Brick::GetRect() const {
    return Rectangle{x, y, width, height};
}

bool Brick::IsDestroyed() const {
    return destroyed;
}

void Brick::Destroy() {
    destroyed = true;
}

//正确写法（关键！！！）
BrickType Brick::GetType() const {
    return type;
}
