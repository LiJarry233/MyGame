#ifndef PADDLE_H
#define PADDLE_H

#include "PhysicalObject.h"
#include "VisualObject.h"

class Paddle : public PhysicalObject, public VisualObject {
private:
    float width, height;

public:
    Paddle(float x, float y, float w, float h)
        : GameObject({x, y}),
          PhysicalObject({x, y}, {0,0}, 0),
          VisualObject(BLUE, true)
    {
        width = w;
        height = h;
    }

    void MoveLeft(float speed) {
        position.x -= speed;
        if (position.x < 5) position.x = 5;
    }

    void MoveRight(float speed) {
        position.x += speed;
        if (position.x + width > 800 - 5)
            position.x = 800 - 5 - width;
    }

    void SetWidth(float newWidth) {
        width = newWidth;
        // 调整位置以保持板子不超出屏幕
        if (position.x + width > 800 - 5) {
            position.x = 800 - 5 - width;
        }
    }
    float GetWidth() const { return width; }

    Rectangle GetRect() const {
        return Rectangle{position.x, position.y, width, height};
    }

    void Draw() {
        DrawRect(width, height);
    }
};

#endif
