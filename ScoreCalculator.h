#pragma once

#include "Brick.h"

class ScoreCalculator {
public:
    // 根据砖块类型加分
    int calculateScore(BrickType type) {
        switch (type) {
        case NORMAL:
            return 10;
        case SPLIT:
            return 20;
        case DOUBLE_SCORE:
            return 30;
        case ENLARGE_PADDLE:
            return 15;
        default:
            return 0;
        }
    }

    // 根据类型和连击数 combo 额外加分
    int calculateScore(BrickType type, int combo) {
        int baseScore = calculateScore(type);
        return baseScore + combo * 5;  // 例如，每连击加5分
    }
};