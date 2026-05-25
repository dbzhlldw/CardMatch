#pragma once
#include <vector>

// 单个槽位定义：位置、z层级、被哪些槽位遮挡（索引）
struct SlotDef {
    float x;
    float y;
    int   zOrder;
    std::vector<int> blockedBy; // 需要先移走这些槽位的牌，此槽才可操作
};

// 一套布局 = 所有桌面槽位的有序列表
// 索引 i 的槽位对应 GameModel::_tableau[i]
using LayoutDef = std::vector<SlotDef>;

// 内置布局
namespace Layouts {
    extern const LayoutDef PYRAMID; // 单峰四行金字塔（10 张桌面牌）
}
