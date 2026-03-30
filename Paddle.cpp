#include "Paddle.h"

Paddle::Paddle(float _x, float _y, float _w, float _h)
    : x(_x), y(_y), width(_w), height(_h) {}

void Paddle::MoveLeft(float speed) {
    x -= speed;
    if (x < 5.0f) x = 5.0f;
}

void Paddle::MoveRight(float speed) {
    x += speed;
    if (x + width > 800 - 5) x = 800 - 5 - width;
}

Rectangle Paddle::GetRect() const {
    return Rectangle{x, y, width, height};
}

void Paddle::Draw() const {
    DrawRectangle((int)x, (int)y, (int)width, (int)height, BLUE);
}
