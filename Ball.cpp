#include "Ball.h"
#include <cmath>

#define PI 3.14159265f
#define DEG2RAD (PI / 180.0f)

Ball::Ball(Vector2 pos, float speed, float angleDeg, float r)
    : position(pos), radius(r)
{
    float rad = angleDeg * DEG2RAD;
    velocity.x = speed * cos(rad);
    velocity.y = -speed * sin(rad);
}

void Ball::Move() {
    position.x += velocity.x;
    position.y += velocity.y;
}

void Ball::BounceEdge(int screenW, int screenH) {
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

void Ball::ReflectX() {
    velocity.x = -velocity.x;
}

void Ball::ReflectY() {
    velocity.y = -fabs(velocity.y);
}

void Ball::Reflect(Vector2 normal) {
    // 归一化法线
    float len = sqrt(normal.x * normal.x + normal.y * normal.y);
    normal.x /= len;
    normal.y /= len;

    float dot = velocity.x * normal.x + velocity.y * normal.y;

    velocity.x = velocity.x - 2 * dot * normal.x;
    velocity.y = velocity.y - 2 * dot * normal.y;
}

void Ball::SetAngle(float angleDeg) {
    float speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    float rad = angleDeg * PI / 180.0f;

    velocity.x = speed * cos(rad);
    velocity.y = -speed * sin(rad);
}

Vector2 Ball::GetPosition() const {
    return position;
}

float Ball::GetRadius() const {
    return radius;
}

void Ball::SetPosition(Vector2 pos) {
    position = pos;
}

void Ball::SetVelocity(Vector2 vel) {
    velocity = vel;
}

Vector2 Ball::GetVelocity() const {
    return velocity;
}

void Ball::Draw() {
    DrawCircleV(position, radius, MAROON);
}