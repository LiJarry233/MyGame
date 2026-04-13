#ifndef BRICK_H
#define BRICK_H

#include "VisualObject.h"

enum BrickType {
    NORMAL,
    SPLIT,
    DOUBLE_SCORE,
    ENLARGE_PADDLE
};

class Brick : public VisualObject {
private:
    float width, height;
    bool destroyed;
    BrickType type;

public:
    Brick(float x, float y, float w, float h, BrickType t)
        : GameObject({x, y}),
          VisualObject(GREEN, true)
    {
        width = w;
        height = h;
        destroyed = false;
        type = t;

        if (type == SPLIT) color = YELLOW;
        if (type == DOUBLE_SCORE) color = PURPLE;
        if (type == ENLARGE_PADDLE) color = PINK;
    }

    void Draw() {
        if (!destroyed)
            DrawRect(width, height);
    }

    Rectangle GetRect() const {
        return Rectangle{position.x, position.y, width, height};
    }

    bool IsDestroyed() const { return destroyed; }

    void Destroy() { destroyed = true; }

    BrickType GetType() const { return type; }
    void SetType(BrickType t) { 
        type = t; 
        // 更新颜色
        if (type == NORMAL) color = GREEN;
        else if (type == SPLIT) color = YELLOW;
        else if (type == DOUBLE_SCORE) color = PURPLE;
        else if (type == ENLARGE_PADDLE) color = PINK;
    }
};

#endif