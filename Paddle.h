#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"

class Paddle {
private:
    float x, y, width, height;

public:
    Paddle(float _x, float _y, float _w, float _h);

    void MoveLeft(float speed);
    void MoveRight(float speed);

    Rectangle GetRect() const;
    void Draw() const;
};

#endif