#pragma once
#include "raylib.h"

class GameObject {
public:
    Vector2 position;

    GameObject(Vector2 pos = {0, 0}) {
        position = pos;
    }
};