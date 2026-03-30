#ifndef BALL_H
#define BALL_H

#include "raylib.h"

class Ball {
private:
    Vector2 position;
    Vector2 velocity;
    float radius;

public:
    Ball(Vector2 pos, float speed, float angleDeg, float r);

    void Move();
    void BounceEdge(int screenW, int screenH);

    void ReflectX();
    void ReflectY();

    void Reflect(Vector2 normal); // 真实反射

    void SetAngle(float angleDeg);

    Vector2 GetPosition() const;
    float GetRadius() const;

    void SetPosition(Vector2 pos);

    void SetVelocity(Vector2 vel);
    Vector2 GetVelocity() const;

    void Draw();
};

#endif