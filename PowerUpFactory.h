#pragma once
#include "PowerUp.h"
#include <vector>
#include <random>

class PowerUpFactory {
private:
    std::vector<PowerUpTypeEnum> availableTypes;
    std::vector<PowerUpTypeEnum> generatedTypes;  // 跟踪已生成的道具类型

public:
    PowerUpFactory() {
        // 初始化所有可用的道具类型
        availableTypes = {
            POWERUP_SPLIT_BALL,
            POWERUP_DOUBLE_SCORE,
            POWERUP_ENLARGE_PADDLE
        };
        generatedTypes.clear();
    }

    // 工厂方法：创建随机类型的道具，确保每种类型都有机会生成
    PowerUp CreateRandomPowerUp(Vector2 position) {
        if (availableTypes.empty()) {
            // 返回一个默认的PowerUp，如果没有可用类型
            return PowerUp(position, POWERUP_SPLIT_BALL);
        }

        // 检查是否有未生成的道具类型
        std::vector<PowerUpTypeEnum> missingTypes;
        for (auto type : availableTypes) {
            bool found = false;
            for (auto generated : generatedTypes) {
                if (generated == type) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                missingTypes.push_back(type);
            }
        }

        PowerUpTypeEnum selectedType;

        // 如果有未生成的类型，优先选择其中一个
        if (!missingTypes.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, missingTypes.size() - 1);
            selectedType = missingTypes[dis(gen)];
        } else {
            // 所有类型都已生成过，正常随机选择
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, availableTypes.size() - 1);
            selectedType = availableTypes[dis(gen)];
        }

        // 记录已生成的类型
        bool alreadyGenerated = false;
        for (auto generated : generatedTypes) {
            if (generated == selectedType) {
                alreadyGenerated = true;
                break;
            }
        }
        if (!alreadyGenerated) {
            generatedTypes.push_back(selectedType);
        }

        return PowerUp(position, selectedType);
    }

    // 工厂方法：创建指定类型的道具
    PowerUp CreatePowerUp(Vector2 position, PowerUpTypeEnum type) {
        return PowerUp(position, type);
    }

    // 获取所有可用道具类型的列表
    const std::vector<PowerUpTypeEnum>& GetAvailableTypes() const {
        return availableTypes;
    }

    // 重置生成记录（用于新一轮游戏）
    void ResetGenerationTracking() {
        generatedTypes.clear();
    }
};