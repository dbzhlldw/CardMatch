// LayoutDef_Pyramid — 金字塔布局的槽位坐标与遮挡关系（Layouts::PYRAMID 的实现）
#include "LayoutDef_Pyramid.h"

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
static const float X0 = 240.f;

static const float Y_BASE = 990.f;
static const float Y_ROW2 = 1210.f;
static const float Y_ROW3 = 1430.f;
static const float Y_APEX = 1650.f;

static float bx(int i) { return X0 + i * S; }
static float mid(float a, float b) { return (a + b) * 0.5f; }

const LayoutDef Layouts::PYRAMID = {
    { bx(0), Y_BASE, 1, {4}    },
    { bx(1), Y_BASE, 1, {4, 5} },
    { bx(2), Y_BASE, 1, {5, 6} },
    { bx(3), Y_BASE, 1, {6}    },

    { mid(bx(0),bx(1)), Y_ROW2, 2, {7}    },
    { mid(bx(1),bx(2)), Y_ROW2, 2, {7, 8} },
    { mid(bx(2),bx(3)), Y_ROW2, 2, {8}    },

    { mid(mid(bx(0),bx(1)), mid(bx(1),bx(2))), Y_ROW3, 3, {9} },
    { mid(mid(bx(1),bx(2)), mid(bx(2),bx(3))), Y_ROW3, 3, {9} },

    { X0 + 1.5f * S, Y_APEX, 4, {} },
};
