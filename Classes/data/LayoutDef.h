// LayoutDef — 桌面布局的类型定义（SlotDef / LayoutDef），不含具体关卡形状
#pragma once
#include <vector>

// 单个槽位：坐标、z 层级、被哪些槽位遮挡（索引）
struct SlotDef {
    float x;
    float y;
    int   zOrder;
    std::vector<int> blockedBy;
};

// 一套桌面布局 = 槽位有序列表；索引 i 对应 GameModel::_tableau[i]
using LayoutDef = std::vector<SlotDef>;
