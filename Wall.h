#pragma once
#include "raylib.h"

class Wall {
private:
    Rectangle rect;
    Color color;

public:
    Wall(float x, float y, float w, float h, Color c = BROWN)
        : rect{x, y, w, h}, color(c) {}

    Rectangle GetRect() const { return rect; }
    Color GetColor() const { return color; }

    void Draw() const {
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 1, Color{100, 60, 20, 255});
    }
};
