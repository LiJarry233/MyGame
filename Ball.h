#ifndef BALL_H 
#define BALL_H

#include "PhysicalObject.h"
#include "VisualObject.h"
#include <cmath>
#include <deque>

class Ball : public PhysicalObject, public VisualObject {
private:
    int scoreValue;

    // trail data
    std::deque<Vector2> trail;
    static const int maxTrail = 15;

public:
    Ball(Vector2 pos, float speed, float angleDeg, float r)
        : GameObject(pos),
          PhysicalObject(pos, {0,0}, r),
          VisualObject(MAROON, true)
    {
        scoreValue = 10;

        float rad = angleDeg * 3.14159265f / 180.0f;
        velocity.x = speed * cos(rad);
        velocity.y = -speed * sin(rad);
    }

    // update trail each frame
    void UpdateTrail() {
        trail.push_front(position);
        if (trail.size() > maxTrail)
            trail.pop_back();
    }

    void BounceEdge(int screenW, int screenH) {
        if (position.x - radius < 5) {
            position.x = 5 + radius;
            Reflect({1, 0});
        }
        if (position.x + radius > screenW - 5) {
            position.x = screenW - 5 - radius;
            Reflect({-1, 0});
        }
        if (position.y - radius < 5) {
            position.y = 5 + radius;
            Reflect({0, 1});
        }
    }

    void ReflectX() { velocity.x = -velocity.x; }
    void ReflectY() { velocity.y = -fabs(velocity.y); }

    void SetAngle(float angleDeg) {
        float speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        float rad = angleDeg * 3.14159265f / 180.0f;

        velocity.x = speed * cos(rad);
        velocity.y = -speed * sin(rad);
    }

    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }

    void SetPosition(Vector2 pos) { position = pos; }
    void SetVelocity(Vector2 vel) { velocity = vel; }
    Vector2 GetVelocity() const { return velocity; }

    // draw (with trail)
    void Draw() {
        // 拖尾
        int i = 0;
        for (auto& pos : trail) {
            float alpha = 1.0f - (float)i / trail.size();
            Color c = Fade(color, alpha * 0.6f);

            DrawCircleV(pos, radius * (1.0f - i * 0.03f), c);
            i++;
        }

        // 本体
        DrawCircleV(position, radius, color);
    }
};

#endif