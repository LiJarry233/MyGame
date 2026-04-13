#ifndef POWERUP_H
#define POWERUP_H

#include "PhysicalObject.h"
#include "VisualObject.h"
#include <cmath>

enum PowerUpTypeEnum {
    POWERUP_SPLIT_BALL,
    POWERUP_DOUBLE_SCORE,
    POWERUP_ENLARGE_PADDLE
};

class PowerUp : public PhysicalObject, public VisualObject {
private:
    PowerUpTypeEnum type;
    float fallSpeed = 2.0f;

public:
    PowerUp(Vector2 pos, PowerUpTypeEnum t)
        : GameObject(pos),
          PhysicalObject(pos, {0, fallSpeed}, 5.0f),
          VisualObject(WHITE, true),
          type(t)
    {
        // 根据类型设置颜色
        if (type == POWERUP_SPLIT_BALL) {
            color = YELLOW;    // 黄色：分裂球
        } else if (type == POWERUP_DOUBLE_SCORE) {
            color = PURPLE;    // 紫色：积分翻倍
        } else if (type == POWERUP_ENLARGE_PADDLE) {
            color = PINK;      // 粉色：板子变长
        }
        radius = 5.0f;  // 比小球小的球形
    }

    void Update() {
        position.y += fallSpeed;
    }

    void Draw() {
        // 绘制光晕效果
        DrawGlow();
        // 绘制道具本体
        DrawCircleV(position, radius, color);
    }

    void DrawGlow() {
        // 光晕效果：多个同心圆，透明度递减
        for (int i = 1; i <= 3; i++) {
            float glowRadius = radius + i * 2.0f;
            float alpha = 0.3f / i;  // 外圈更透明
            Color glowColor = Fade(color, alpha);
            DrawCircleV(position, glowRadius, glowColor);
        }
    }

    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    PowerUpTypeEnum GetType() const { return type; }

    bool IsFallen(int screenHeight) const {
        return position.y - radius > screenHeight;
    }
};

#endif
