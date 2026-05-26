// CardAppearance — 各 CardKind 的显示配置
#pragma once
#include "data/CardDef.h"

struct CardVisualStyle {
    bool        useStandardPips = true; // 使用 suit/value 数字图
    const char* overlayImage    = nullptr; // 非空时叠在牌面中央（如万能牌图标）
};

const CardVisualStyle& cardVisualStyleFor(CardKind kind);
