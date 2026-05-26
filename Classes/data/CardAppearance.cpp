// CardAppearance — 各 CardKind 的显示配置
#include "CardAppearance.h"

const CardVisualStyle& cardVisualStyleFor(CardKind kind) {
    static const CardVisualStyle kNormal { true, nullptr };
    switch (kind) {
        case CardKind::Normal:
        default:
            return kNormal;
    }
}
