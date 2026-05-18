#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"

struct Particle {
    Vector2 pos;
    Vector2 vel;
    float life;
    float invMaxLife;   // precomputed 1/maxLife for cheaper alpha
    Color color;

    Particle(Vector2 p) {
        pos = p;
        vel = { (float)GetRandomValue(-100,100)/50.0f,
                (float)GetRandomValue(-100,100)/50.0f };
        float maxLife = (float)GetRandomValue(20, 40);
        life = maxLife;
        invMaxLife = 1.0f / maxLife;
        color = ORANGE;
    }

    // returns true while still alive
    bool Update() {
        pos.x += vel.x;
        pos.y += vel.y;
        vel.y += 0.05f; // gravity
        life -= 1.0f;
        return life > 0.0f;
    }

    void Draw() {
        float alpha = life * invMaxLife;
        // Use a 4x4 filled rect instead of DrawCircleV: a 3px circle in
        // raylib still issues a 36-segment triangle fan. A small rect is
        // one quad — orders of magnitude fewer vertices per particle.
        Color c = Fade(color, alpha);
        DrawRectangle((int)pos.x - 2, (int)pos.y - 2, 4, 4, c);
    }

    bool IsDead() const {
        return life <= 0.0f;
    }
};

#endif