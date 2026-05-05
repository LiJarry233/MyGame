#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"

struct Particle {
    Vector2 pos;
    Vector2 vel;
    float life;
    float maxLife;
    Color color;

    Particle(Vector2 p) {
        pos = p;
        vel = { (float)GetRandomValue(-100,100)/50.0f,
                (float)GetRandomValue(-100,100)/50.0f };
        life = maxLife = (float)GetRandomValue(20, 40);
        color = ORANGE;
    }

    void Update() {
        pos.x += vel.x;
        pos.y += vel.y;
        vel.y += 0.05f; // ⭐ 重力（更真实）
        life--;
    }

    void Draw() {
        float alpha = life / maxLife;
        DrawCircleV(pos, 3, Fade(color, alpha));
    }

    bool IsDead() const {
        return life <= 0;
    }
};

#endif