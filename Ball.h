#ifndef BALL_H
#define BALL_H

#include "PhysicalObject.h"
#include "VisualObject.h"
#include <cmath>

class Ball : public PhysicalObject, public VisualObject {
private:
    int scoreValue;

    // Trail as a fixed-size ring buffer to avoid per-frame heap traffic
    // that std::deque<Vector2>::push_front used to incur. Trail length
    // reduced from 15 -> 8: each ball used to push 15 DrawCircleV per
    // frame, which is the dominant CPU draw-batch cost when many balls
    // are alive.
    static const int maxTrail = 8;
    Vector2 trail[maxTrail];
    int     trailHead = 0;   // next write index
    int     trailLen  = 0;   // current size (<= maxTrail)

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

    void UpdateTrail() {
        trail[trailHead] = position;
        trailHead = (trailHead + 1) % maxTrail;
        if (trailLen < maxTrail) trailLen++;
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
        // Iterate from newest to oldest using the ring buffer.
        if (trailLen > 0) {
            float invLen = 1.0f / (float)trailLen;
            for (int i = 0; i < trailLen; i++) {
                int idx = trailHead - 1 - i;
                if (idx < 0) idx += maxTrail;
                float alpha = 1.0f - i * invLen;
                Color c = Fade(color, alpha * 0.6f);
                DrawCircleV(trail[idx], radius * (1.0f - i * 0.03f), c);
            }
        }

        DrawCircleV(position, radius, color);
    }

    void update() override {
        Move();
        UpdateTrail();
    }

    void draw() override {
        Draw();
    }
};

#endif