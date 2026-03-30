#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

enum BrickType {
    NORMAL,
    SPLIT,
    DOUBLE_SCORE
};

class Brick {
private:
    float x, y, width, height;
    bool destroyed;
    BrickType type;   // 一定要有

public:
    Brick(float _x, float _y, float _w, float _h, BrickType _type);

    void Draw() const;
    Rectangle GetRect() const;
    bool IsDestroyed() const;
    void Destroy();

    BrickType GetType() const;  // 一定要声明
};

#endif