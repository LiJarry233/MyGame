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
        // 保证位置不出界
        if (position.x + width > 800 - 5) {
            position.x = 800 - 5 - width;
        }
    }
    float GetWidth() const { return width; }

    void SetPosition(const Vector2& pos) {
        position = pos;
        if (position.x < 5) position.x = 5;
        if (position.x + width > 800 - 5) position.x = 800 - 5 - width;
    }

    Rectangle GetRect() const {
        return Rectangle{position.x, position.y, width, height};
    }

    void Draw() {
        DrawRect(width, height);
    }

    void update() override {
        // Paddle update logic if needed
    }

    void draw() override {
        Draw();
    }
};

#endif
