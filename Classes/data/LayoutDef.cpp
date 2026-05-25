#include "LayoutDef.h"

// ---------------------------------------------------------------------------
// 单峰四行金字塔布局（10 张桌面牌）
//
// 遮挡方向：上层牌压在下层牌上面，因此下层被遮挡。
// 顶点最先可操作，逐层向下解锁。
//
//   槽位 9     顶点（Apex，1 张），z=4，无遮挡 → 初始可点
//   槽位 7-8   第 3 行（Row3，2 张），z=3，各被 Apex 遮挡
//   槽位 4-6   第 2 行（Row2，3 张），z=2，被相邻 Row3 牌遮挡
//   槽位 0-3   底行（Base，4 张）， z=1，被相邻 Row2 牌遮挡
//
// 视觉排列（屏幕 y 越大越靠上）：
//
//              [9]               y=1450  ← 可操作
//           [7]  [8]             y=1230  ← 被 9 遮挡
//        [4]  [5]  [6]           y=1010  ← 被 7/8 遮挡
//     [0]  [1]  [2]  [3]         y= 790  ← 被 4/5/6 遮挡
//
// X 坐标：底行间距 200px，整体居中于 1080px 宽度（startX=240）。
// ---------------------------------------------------------------------------

static const float S  = 200.f;
static const float X0 = 240.f;  // 底行第一张 x

static const float Y_BASE = 990.f;
static const float Y_ROW2 = 1210.f;
static const float Y_ROW3 = 1430.f;
static const float Y_APEX = 1650.f;

static float bx(int i) { return X0 + i * S; }
static float mid(float a, float b) { return (a + b) * 0.5f; }

const LayoutDef Layouts::PYRAMID = {
    // ---- 底行 Slots 0-3（z=1，被相邻 Row2 牌遮挡）----
    // Base[0] 被 Row2[0](slot4) 遮挡
    { bx(0), Y_BASE, 1, {4}    },  // 0
    // Base[1] 被 Row2[0](slot4) 和 Row2[1](slot5) 遮挡
    { bx(1), Y_BASE, 1, {4, 5} },  // 1
    // Base[2] 被 Row2[1](slot5) 和 Row2[2](slot6) 遮挡
    { bx(2), Y_BASE, 1, {5, 6} },  // 2
    // Base[3] 被 Row2[2](slot6) 遮挡
    { bx(3), Y_BASE, 1, {6}    },  // 3

    // ---- 第 2 行 Slots 4-6（z=2，被相邻 Row3 牌遮挡）----
    // Row2[0] 被 Row3[0](slot7) 遮挡
    { mid(bx(0),bx(1)), Y_ROW2, 2, {7}    },  // 4
    // Row2[1] 被 Row3[0](slot7) 和 Row3[1](slot8) 遮挡
    { mid(bx(1),bx(2)), Y_ROW2, 2, {7, 8} },  // 5
    // Row2[2] 被 Row3[1](slot8) 遮挡
    { mid(bx(2),bx(3)), Y_ROW2, 2, {8}    },  // 6

    // ---- 第 3 行 Slots 7-8（z=3，被 Apex(slot9) 遮挡）----
    { mid(mid(bx(0),bx(1)), mid(bx(1),bx(2))), Y_ROW3, 3, {9} },  // 7
    { mid(mid(bx(1),bx(2)), mid(bx(2),bx(3))), Y_ROW3, 3, {9} },  // 8

    // ---- 顶点 Slot 9（z=4，无遮挡，初始可操作）----
    { X0 + 1.5f * S, Y_APEX, 4, {} },  // 9  x=540
};
