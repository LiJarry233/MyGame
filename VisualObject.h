#pragma once
#include "GameObject.h"
#include "raylib.h"

class VisualObject : virtual public GameObject {
public:
    Color color;
    bool visible;

    VisualObject(Color c = WHITE, bool v = true)
        : GameObject()
    {
        color = c;
        visible = v;
    }

    void DrawCircle(float radius) {
        if (visible)
            DrawCircleV(position, radius, color);
    }

    void DrawRect(float w, float h) {
        if (visible)
            DrawRectangle((int)position.x, (int)position.y, (int)w, (int)h, color);
    }
};