#pragma once
#include "GameObject.h"
#include <cmath>

class PhysicalObject : virtual public GameObject {
public:
    Vector2 velocity;
    float radius;

    PhysicalObject(Vector2 pos = {0,0}, Vector2 vel = {0,0}, float r = 10.0f)
        : GameObject(pos)
    {
        velocity = vel;
        radius = r;
    }

    void Move() {
        position.x += velocity.x;
        position.y += velocity.y;
    }

    void Reflect(Vector2 normal) {
        float len = sqrt(normal.x * normal.x + normal.y * normal.y);
        normal.x /= len;
        normal.y /= len;

        float dot = velocity.x * normal.x + velocity.y * normal.y;

        velocity.x -= 2 * dot * normal.x;
        velocity.y -= 2 * dot * normal.y;
    }
};